/* A player plugin driver for linux-epuck (lpuck)
 *
 * started by Wenguo Liu
 *
 * 16/09/2008: initial version
 * 20/09/2008: added SPI communication
 * 18/10/2008: added power infterface
 * 01/07/2009: add options "load_gps" "batt_factor"
 *
 *
 *
 *-----------------------------------------------------------------
 *  driver
 *  (
 *    name "lpuck"
 *    plugin "liblpuck"
 *    provides ["position2d:0" "camera:0" "ir:0" "power:0"]
 *    load_gps 0
 *    batt_factor 3.0
 *    camera_device "/dev/video0"
 *    image_size [640 480]
 *    save_frame 1
 *   )
 *
 *----------------------------------------------------------------
 *
 *TODO:
 *    1. fix the SPI bus communication
 *    2. implement the I2C slave in dsPIC
 *    3. intergrate with VICON system
 *    4. build a table for battery
 *
 * */


#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <linux/videodev2.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

#include "lpuck.h"
//#include "gps_client.h"

#ifndef V4L2_PIX_FMT_UYVY
#define V4L2_PIX_FMT_UYVY     v4l2_fourcc('U','Y','V','Y') /* 16 YUV 4:2:2 */
#endif

#define  WHEEL_SEP   0.052
#define IR_COUNT 8
#define AMB_COUNT 8
#define MIC_COUNT 3


//#define TEST 1

struct ir_pose_t ir_pose[] =
{
  { 0.030, -0.010, 342.8},
  { 0.022, -0.025, 314.2},
  {   0.0, -0.031,   270},
  { -0.03, -0.015, 208.5},
  { -0.03,  0.015, 151.5},
  {   0.0,  0.031,    90},
  { 0.022,  0.025, 405.8},
  {  0.03,   0.01, 377.2},
};


static unsigned char clip(int value)
{
  if (value < 0)
    value = 0;
  else if (value > 255)
    value = 255;
  return value;
}

static void yuv422_to_rgb(unsigned char *src, unsigned char *dst,
                          unsigned int nr_pixels)
{
  unsigned int i;
  int y1, y2, u, v;

  for (i = 0; i < nr_pixels; i += 2)
  {
    /* Input format is Cb(i)Y(i)Cr(i)Y(i+1) */
    u = *src++;
    y1 = *src++;
    v = *src++;
    y2 = *src++;
    y1 -= 16;
    u -= 128;
    v -= 128;
    y2 -= 16;

    *dst++ = clip(( 298 * y1           + 409 * v + 128) >> 8);
    *dst++ = clip(( 298 * y1 - 100 * u - 208 * v + 128) >> 8);
    *dst++ = clip(( 298 * y1 + 516 * u           + 128) >> 8);

    *dst++ = clip(( 298 * y2           + 409 * v + 128) >> 8);
    *dst++ = clip(( 298 * y2 - 100 * u - 208 * v + 128) >> 8);
    *dst++ = clip(( 298 * y2 + 516 * u           + 128) >> 8);
  }
}
////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class LPuck : public Driver
{
public:

  // Constructor; need that
  LPuck(ConfigFile* cf, int section);

  // Must implement the following methods.
  virtual int Setup();
  virtual int Shutdown();

  // This method will be invoked on each incoming message
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);

  // these handle subscription, toggling motor state.
  virtual int Subscribe(player_devaddr_t addr);
  virtual int Unsubscribe(player_devaddr_t addr);

private:

  // Main function for device thread.
  virtual void Main();

  int initCamera();
  int closeCamera();

  int grabFrame();

  void refreshCameraData();
  void refreshIRData();
  void refreshPowerData();
  void refreshPosData();
  void refreshAIOData();
  void refreshLEDData();

  int saveFrame(const unsigned char * buf, const char * filename);

  //for SPI
  int initSPI();
  int closeSPI();
  const char *spi_device;
  int spi_fd;

  void doMSG(int16_t *txbuf, int16_t *rxbuf, int16_t len);
  struct txbuf_t msgTX; //data to dsPIC
  struct rxbuf_t msgRX; //data from dsPIC

  //build up the msgTX data
  void setSpeedCMD(int16_t leftspeed, int16_t rightspeed);


private:

  // My position interface
  player_devaddr_t position_id;
  // My camera interface
  player_devaddr_t camera_id;
  // My IR proximity sensor
  player_devaddr_t ir_id;
  // My LEDs
  player_devaddr_t led_id;
  // My Battery interface
  player_devaddr_t power_id;
  // My aio interface (ADC : microphone and ambient IR)
  player_devaddr_t aio_id;


  int camera_fd;
  struct image_t imageFrame;
  int width;  //image width
  int height;  //image height
  int save;  //save frame to *.ppm
  const char *cam_device; //video device
  int load_gps; //load gps data from vicon
  float batt_factor;

  int frameno;
  char filename[64];

  int publish_interval;

  // Capture timestamp
  uint32_t tsec, tusec;
  time_t publish_time;

  // its best to maintain a subscription count
  // to enable and disable motors when people connect
  int position_subscriptions;
  int ir_subscriptions;
  int power_subscriptions;
  int aio_subscriptions;
  int blinkenlight_subscriptions;

  // used to keep memory of what led's are to function, as the
  // player interface only addresses 1 light per message
  int led_status[10];



};

Driver*
LPuck_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return((Driver*)(new LPuck(cf, section)));
}

void LPuck_Register(DriverTable* table)
{
  table->AddDriver("lpuck", LPuck_Init);
}

LPuck::LPuck(ConfigFile* cf, int section)
    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN )
{

  memset(&this->position_id, 0, sizeof(player_devaddr_t));
  memset(&this->camera_id, 0, sizeof(player_devaddr_t));
  memset(&this->ir_id, 0, sizeof(player_devaddr_t));
  memset(&this->led_id, 0, sizeof(player_devaddr_t));
  memset(&this->power_id, 0, sizeof(player_devaddr_t));
  memset(&this->aio_id, 0, sizeof(player_devaddr_t));

  this->position_subscriptions = 0;
  this->power_subscriptions = 0;
  this->ir_subscriptions = 0;
  this->aio_subscriptions = 0;
  this->blinkenlight_subscriptions = 0;


  // Create my position interface
  if (cf->ReadDeviceAddr(&(this->position_id), section, "provides",
                         PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->position_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Create my camera interface
  if (cf->ReadDeviceAddr(&(this->camera_id), section, "provides",
                         PLAYER_CAMERA_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->camera_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Create my ir interface
  if (cf->ReadDeviceAddr(&(this->ir_id), section, "provides",
                         PLAYER_IR_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->ir_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Create my led interface
  if (cf->ReadDeviceAddr(&(this->led_id), section, "provides",
                         PLAYER_BLINKENLIGHT_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->led_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Create my power interface
  if (cf->ReadDeviceAddr(&(this->power_id), section, "provides",
                         PLAYER_POWER_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->power_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Create my aio interface
  if (cf->ReadDeviceAddr(&(this->aio_id), section, "provides",
                         PLAYER_AIO_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->aio_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  //read configuation from file
  this->width = cf->ReadTupleInt(section, "image_size", 0, this->width);
  this->height = cf->ReadTupleInt(section, "image_size", 1, this->height);

  this->save = cf->ReadInt(section, "save_frame", 0);
  this->cam_device = cf->ReadString(section, "camera_device", "/dev/video0");

  this->spi_device = cf->ReadString(section, "spi_device", "/dev/spidev1.0");

  this->load_gps = cf->ReadInt(section, "load_gps", 0);

  this->batt_factor = cf->ReadFloat(section, "batt_factor", 2.25);


  this->camera_fd = -1;
  this->imageFrame.buf = NULL;

  this->spi_fd = -1;

  initSPI();

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int LPuck::Setup()
{
  puts("LPuck driver initialising");

  if (this->camera_id.interf)
  {
    initCamera();
  }

  // Start the device thread; spawns a new thread and executes
  // LPuck::Main(), which contains the main loop for the driver.
  StartThread();

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int LPuck::Shutdown()
{
  puts("Shutting LPuck driver down");

  // Stop and join the driver thread
  StopThread();

  // Here you would shut the device down by, for example, closing a
  // serial port.
  closeCamera();

  puts("LPuck driver has been shutdown");

  return(0);
}

int LPuck::ProcessMessage(QueuePointer & resp_queue,
                          player_msghdr * hdr,
                          void * data)
{
//printf("Message In: T: %d  ST: %d \n", hdr->type, hdr->subtype);
  // check for cap checks first
  HANDLE_CAPABILITY_REQUEST (position_id, resp_queue, hdr, data, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ);
  HANDLE_CAPABILITY_REQUEST (position_id, resp_queue, hdr, data, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL);

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, this->position_id))
  {
    player_position2d_cmd_vel_t * poscmd = reinterpret_cast<player_position2d_cmd_vel_t *> (data);

    // need to calculate the left and right velocities
    int transvel = static_cast<int> (static_cast<int> (poscmd->vel.px * 1000 / 0.125));
    int rotvel = static_cast<int> (static_cast<int> (poscmd->vel.pa * WHEEL_SEP / 2 * (1000 / 0.125)));
    int leftvel = transvel - rotvel;
    int rightvel = transvel + rotvel;

    // now we set the speed
    if (leftvel > 800)
      leftvel = 800;
    if (leftvel < -800)
      leftvel = -800;
    if (rightvel > 800)
      rightvel = 800;
    if (rightvel < -800)
      rightvel = -800;

    setSpeedCMD(leftvel, rightvel);
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, position_id))
  {
    player_position2d_geom_t geom;
    geom.pose.px = 0.0;
    geom.pose.py = 0.0;
    geom.pose.pz = 0.0;

    geom.size.sl = 0.53;  // 53 mm.
    geom.size.sw = 0.53;
    //publish
    this->Publish(this->position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_GET_GEOM,  (void*)&geom, sizeof(geom), NULL);
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_IR_REQ_POSE, this->ir_id))
  {
    printf("ir pose\n");

    /* Return the sonar geometry. */
    if (hdr->size != 0)
    {
      PLAYER_WARN("Arg get ir pose is wrong size; ignoring");
      return(-1);
    }
    player_ir_pose_t geom;
    geom.poses_count = IR_COUNT;
    geom.poses = new player_pose3d_t[geom.poses_count];
    for (int i = 0; i < IR_COUNT; i++)
    {
      ir_pose_t pose = ir_pose[i];
      geom.poses[i].px = pose.x ;
      geom.poses[i].py = pose.y ;
      geom.poses[i].pyaw = 3.141*pose.th/180; //DTOR(pose.th);
      printf("pose: %f %f %f\n", geom.poses[i].px, geom.poses[i].py, geom.poses[i].pyaw);
    }

    this->Publish(this->ir_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_IR_REQ_POSE,
                  (void*)&geom);
    delete [] geom.poses;
    return(0);

  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BLINKENLIGHT_CMD_POWER, this->led_id))
  {
  	//cast data to power command
  	player_blinkenlight_cmd_power_t* rec = (player_blinkenlight_cmd_color_t*)data;
  	//what state should the led be... 
	uint8_t newstate = rec->enable;

	//if a number that is too large is given then set all ring leds to the given state
	if(rec->id > 10)
	{
		int count;
		for(count=0; count<8; count++)
		{
			led_status[count] = newstate;
		}
	}
  	//update saved list of LED states
  	else led_status[rec->id] = newstate;
  	//send acknowledgement
  	this->Publish(this->led_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_BLINKENLIGHT_CMD_POWER);
  }

  return(0);
}



////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void LPuck::Main()
{
  int last_position_subscrcount = 0;
  this->publish_time = 0;
  this->frameno = 0;


  // The main loop; interact with the device here
  for (;;)
  {


    // Process incoming messages.  LPuck::ProcessMessage() is
    // called on each message.
    ProcessMessages();

    // Interact with the device, and push out the resulting data, using
    // Driver::Publish()
    if (camera_id.interf)
      refreshCameraData();

    if (ir_id.interf && ir_subscriptions > 0)
    {
      refreshIRData();
    }
    if (power_id.interf && power_subscriptions > 0)
    {
      refreshPowerData();
    }
    if (position_id.interf && position_subscriptions > 0 )
    {
      refreshPosData();
    }
    if (aio_id.interf && aio_subscriptions > 0 )
    {
      refreshAIOData();
    }
	if (led_id.interf && blinkenlight_subscriptions > 0 )
    {
      refreshLEDData();
    }
    

    if (!last_position_subscrcount && this->position_subscriptions)
    {
      printf("LPuck: Subscription; zero speed\n");
      setSpeedCMD(0, 0);

    }
    else if (last_position_subscrcount && !(this->position_subscriptions))
    {
      printf("LPuck: Unsubscribe: zero speed\n");
      setSpeedCMD(0, 0);
    }
    last_position_subscrcount = this->position_subscriptions;



    //     doMSG((int16_t *)&msgTX, (int16_t *)&msgRX, 8);
    //doMSG((int16_t *)&msgTX, (int16_t *)&msgRX, sizeof(msgTX) / sizeof(int16_t));
    doMSG((int16_t *)&msgTX, (int16_t *)&msgRX, sizeof(msgTX) / sizeof(int16_t));


    // test if we are supposed to cancel
    pthread_testcancel();

    // Sleep (you might, for example, block on a read() instead)
    usleep(100000);


  }
}


int LPuck::Subscribe(player_devaddr_t addr)
{
  int setupResult;

  // do the subscription
  if ((setupResult = Driver::Subscribe(addr)) == 0)
  {
    // also increment the appropriate subscription counter
    switch (addr.interf)
    {
      case PLAYER_POSITION2D_CODE:
        this->position_subscriptions++;;
        printf("Subscribe to Position2D interface, total %d\n", this->position_subscriptions);
        break;
      case PLAYER_IR_CODE:
        this->ir_subscriptions++;
        printf("Subscribe to IR interface, total %d\n", this->ir_subscriptions);
        break;
      case PLAYER_POWER_CODE:
        this->power_subscriptions++;
        printf("Subscribe to POWER interface, total %d\n", this->power_subscriptions);
        break;
      case PLAYER_BLINKENLIGHT_CODE:
        this->blinkenlight_subscriptions++;
        printf("Subscribe to LED interface, total %d\n", this->blinkenlight_subscriptions);
        break;
      case PLAYER_AIO_CODE:
        this->aio_subscriptions++;
        printf("Subscribe to AIO interface, total %d\n", this->aio_subscriptions);
        break;
    }
  }

  return(setupResult);
}

int LPuck::Unsubscribe(player_devaddr_t addr)
{
  int shutdownResult;
  // do the unsubscription
  if ((shutdownResult = Driver::Unsubscribe(addr)) == 0)
  {
    // also decrement the appropriate subscription counter
    switch (addr.interf)
    {
      case PLAYER_POSITION2D_CODE:
        assert(--this->position_subscriptions >= 0);
        printf("Unsubscribe to Position2D interface, total %d\n", this->position_subscriptions);
        break;
      case PLAYER_IR_CODE:
        assert(--this->ir_subscriptions >= 0);
        printf("Unsubscribe to IR interface, total %d\n", this->ir_subscriptions);
        break;
      case PLAYER_POWER_CODE:
        assert(--this->power_subscriptions >= 0);
        printf("Unsubscribe to POWER interface, total %d\n", this->power_subscriptions);
        break;
      case PLAYER_BLINKENLIGHT_CODE:
        assert(--this->blinkenlight_subscriptions >= 0);
        printf("Unsubscribe to LED interface, total %d\n", this->blinkenlight_subscriptions);
        break;
      case PLAYER_AIO_CODE:
        assert(--this->aio_subscriptions >= 0 );
        printf("Unsubscribe to AIO interface, total %d\n", this->aio_subscriptions);
        break;
    }
  }
  return(shutdownResult);
}

int LPuck::initCamera()
{

  struct v4l2_capability cap;
  struct v4l2_format fmt;

  this->camera_fd = open(this->cam_device, O_RDONLY);

  if (this->camera_fd < 0)
  {
    fprintf(stderr, "Can not open %s\n", this->cam_device);
    return -1;
  }


  if (ioctl(this->camera_fd, VIDIOC_QUERYCAP, &cap) < 0)
    return (-1);

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
  {
    fprintf(stderr, "No video capture capability present\n");
    return (-1);
  }
  if (!(cap.capabilities & V4L2_CAP_READWRITE))
  {
    fprintf(stderr, "read() interface not supported by driver\n");
    return (-1);
  }

  /* Select video format */
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = this->width;
  fmt.fmt.pix.height = this->height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

  if (ioctl(this->camera_fd, VIDIOC_S_FMT, &fmt))
  {
    fprintf(stderr, "VIDIOC_S_FMT failed");
    return (-1);
  }

  this->imageFrame.width = fmt.fmt.pix.width;
  this->imageFrame.height = fmt.fmt.pix.height;
  this->imageFrame.bytes_per_line = fmt.fmt.pix.bytesperline;
  this->imageFrame.size = fmt.fmt.pix.sizeimage;
  this->imageFrame.buf = malloc(this->imageFrame.size);
  if (!this->imageFrame.buf)
  {
    fprintf(stderr, "Out of memory\n");
    return (-1);
  }

  return 0;
}

int LPuck::initSPI()
{
  static uint8_t mode = 1;
  static uint8_t bits = 16;
  static uint32_t speed = 20000000;
  this->spi_fd = open(this->spi_device, O_RDWR);
  if (this->spi_fd < 0)
  {
    fprintf(stderr, "can't open device");
    return -1;
  }

  /*
  *          * spi mode
  *                   */
  int ret;
  ret = ioctl(this->spi_fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
  {
    fprintf(stderr, "can't set spi mode");
  }

  ret = ioctl(this->spi_fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
  {
    fprintf(stderr, "can't get spi mode");
    return -1;
  }

  /*
  *          * bits per word
  *                   */
  ret = ioctl(this->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
  {
    fprintf(stderr, "can't set bits per word");
    return -1;
  }

  ret = ioctl(this->spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
  {
    fprintf(stderr, "can't get bits per word");
    return -1;
  }

  /*
   *          * max speed hz
   *                   */
  ret = ioctl(this->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
  {
    fprintf(stderr, "can't set max speed hz");
    return -1;
  }

  ret = ioctl(this->spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
  {
    fprintf(stderr, "can't get max speed hz");
    return -1;
  }

  return 0;
}


int LPuck::closeCamera()
{
  if (this->camera_fd > 0)
  {
    close(this->camera_fd);
  }

  if (this->imageFrame.buf)
  {
    free(this->imageFrame.buf);
    this->imageFrame.buf = NULL;
  }
  return 0;
}

int LPuck::closeSPI()
{
  if (this->spi_fd > 0)
  {
    close(this->spi_fd);
  }
  return 0;
}

int LPuck::grabFrame()
{
  unsigned int nbytes = this->imageFrame.size;
  ssize_t ret;

  while (nbytes > 0)
  {
    ret = read(this->camera_fd, this->imageFrame.buf, nbytes);
    if (ret < 0)
      return -1;
    nbytes -= ret;
  }

  return 0;
}

void LPuck::refreshIRData()
{
  player_ir_data_t ir_data;
  ir_data.ranges_count = IR_COUNT;
  ir_data.voltages_count = IR_COUNT;
  ir_data.ranges = new float[ir_data.ranges_count];
  ir_data.voltages = new float[ir_data.voltages_count];

  for (int i = 0;i < IR_COUNT;i++)
  {
    ir_data.ranges[i] = (double)msgRX.ir[i];
    //printf("%.2f ", (double)msgRX.ir[i]);
  }
  //printf("\n");

  Publish(this->ir_id, PLAYER_MSGTYPE_DATA, PLAYER_IR_DATA_RANGES, (void *) &ir_data, sizeof(player_ir_data_t), NULL);
  player_ir_data_t_cleanup(&ir_data);
}

void LPuck::refreshPowerData()
{
  player_power_data_t power_data;
  power_data.valid = 1;
  //FIXME: find  correct  coefficients for the voltage conversion
  //power_data.volts = this->batt_factor * 3.3 * (msgRX.batt * 1.0) / 4096;
  power_data.percent = 0;
  power_data.joules = msgRX.batt * 1.0;
  power_data.watts = 0;
  power_data.charging = 0;
//    printf("battery: %d %f\n", msgRX.batt, power_data.volts);
  Publish(this->power_id, PLAYER_MSGTYPE_DATA, PLAYER_POWER_DATA_STATE, (void *) &power_data, sizeof(player_power_data_t), NULL);
  player_power_data_t_cleanup(&power_data);

}

// Update position2d data from VICON gps system
// accelerometer added in as velocities.
/*void LPuck::refreshPosData()
{
  player_position2d_data_t posdata;

  memset(&posdata, 0, sizeof(posdata));

  // create our gps data struct and get values
  if (this->load_gps)
  {
    gps_position_t gps_values;
    getGPS( &gps_values, 0);

    posdata.pos.px = (double)(gps_values.px) / 10;
    posdata.pos.py = (double)(gps_values.py) / 10;
    posdata.pos.pa = (double)(gps_values.pa) / 1000;
  }
  else
  {
    posdata.pos.px = 0;
    posdata.pos.py = 0;
    posdata.pos.pa = 0;
  }
  posdata.vel.px = (double)msgRX.acc[0];
  posdata.vel.py = (double)msgRX.acc[1];
  posdata.vel.pa = (double)msgRX.acc[2];

  this->Publish(this->position_id, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, (void*)&posdata, sizeof(posdata), NULL);
}*/

void LPuck::refreshAIOData()
{
  int i;

  player_aio_data_t aio_data;

  aio_data.voltages_count = MIC_COUNT + AMB_COUNT;
  aio_data.voltages = new float[aio_data.voltages_count];

  // microphones.
  for ( i = 0; i < MIC_COUNT; i++ )
  {
    aio_data.voltages[i] = (double)msgRX.mic[i];
    //printf("%.2f, ", aio_data.voltages[i]);
  }
  //printf("\n");

  // ambient ir values.
  for ( i = 0; i < AMB_COUNT; i++ )
  {
    aio_data.voltages[ MIC_COUNT + i] = (double)msgRX.amb[i];
    //printf("%.2f, ", aio_data.voltages[MIC_COUNT + i]);
  }
  //printf("\n");

  this->Publish(this->aio_id, PLAYER_MSGTYPE_DATA, PLAYER_AIO_DATA_STATE, (void*)&aio_data, sizeof(aio_data), NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void LPuck::refreshLEDData()
{
	msgTX.led_cmd.led0 = led_status[0];
	msgTX.led_cmd.led1 = led_status[1];
	msgTX.led_cmd.led2 = led_status[2];
	msgTX.led_cmd.led3 = led_status[3];
	msgTX.led_cmd.led4 = led_status[4];
	msgTX.led_cmd.led5 = led_status[5];
	msgTX.led_cmd.led6 = led_status[6];
	msgTX.led_cmd.led7 = led_status[7];
	msgTX.led_cmd.bodyled = led_status[8];
	msgTX.led_cmd.frontled = led_status[9];
	return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void LPuck::refreshCameraData()
{

  player_camera_data_t * data = reinterpret_cast<player_camera_data_t *>(malloc(sizeof(player_camera_data_t)));

  if (!data)
  {
    PLAYER_ERROR("Out of memory!");
    return;
  }

  // Set the image properties
  data->width       = this->width;
  data->height      = this->height;
  data->bpp         = 24;
  data->format      = PLAYER_CAMERA_FORMAT_RGB888;
  data->fdiv        = 0;
  data->compression = PLAYER_CAMERA_COMPRESS_RAW;


  //allocate new memory

  data->image_count = this->width * this->height * 3;
  data->image = reinterpret_cast<uint8_t *>(malloc(data->image_count));
  if (!(data->image))
  {
    PLAYER_ERROR("Out of memory!");
    free(data);

    return;
  }

  //grab the frame
  grabFrame();

  this->frameno++;

  //convert and copy data to data->image
  yuv422_to_rgb((unsigned char *)this->imageFrame.buf, data->image, this->width * this->height);

  if (this->save)
  {
    snprintf(this->filename, sizeof(this->filename), "click-%04d.ppm", frameno);
    saveFrame(data->image, this->filename);
  }

//    usleep(2000000);

  //publish data
  if (this->publish_interval)
  {
    if ((time(NULL) - (this->publish_time)) < (this->publish_interval))
    {
      data->width       = 0;
      data->height      = 0;
      data->bpp         = 0;
      data->image_count = 0;
      free(data->image);
      data->image       = NULL;
    }
    else this->publish_time = time(NULL);
  }

  Publish(this->camera_id,
          PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
          reinterpret_cast<void *>(data), 0, NULL, false);

}

int LPuck::saveFrame(const unsigned char * buf, const char * fname)
{
  FILE * fp = fopen(fname, "w");

  if ( fp == NULL )
  {
    perror( "frame_save(): opening file for writing" );
    return -1;
  }

  // Write PNM header
  fprintf( fp, "P6\n" );
  fprintf( fp, "# Generated by a herd of rabid fleas\n" );
  fprintf( fp, "%d %d 255\n", this->width, this->height );

  for (int i = 0;i < this->width * this->height * 3;i += 3)
  {
    fprintf(fp, "%c%c%c", buf[i], buf[i+1], buf[i+2]);
  }
  fclose(fp);

  return 0;

}


void LPuck::doMSG(int16_t *txbuf, int16_t *rxbuf, int16_t len)
{
  struct spi_ioc_transfer xfer;

  int   status;

  memset(&xfer, 0, sizeof xfer);
  memset(rxbuf, 0, sizeof rxbuf);

  xfer.rx_buf = (__u64) rxbuf;
  xfer.tx_buf = (__u64) txbuf;
  xfer.len = 2 * len; //size in bytes


  status = ioctl(this->spi_fd, SPI_IOC_MESSAGE(1), &xfer);
  if (status < 0)
  {
    fprintf(stderr, "SPI_IOC_MESSAGE");
    memset(rxbuf, 0, sizeof rxbuf);
    return;
  }

#ifdef TEST
  int16_t *bp;
  int16_t len_tmp = len;
  printf("send(%2d, %2d): ", 2* len_tmp, status);
  for (bp = txbuf; len_tmp; len_tmp--)
    printf(" %d", *bp++);
  printf("\n");

  len_tmp = len;
  printf("response(%2d, %2d): ", 2* len_tmp, status);
  for (bp = rxbuf; len_tmp; len_tmp--)
    printf(" %d", *bp++);
  printf("\n");
#endif

  memset(txbuf, 0, sizeof(txbuf));
}

void LPuck::setSpeedCMD(int16_t leftspeed, int16_t rightspeed)
{
  msgTX.cmd.set_motor = 1;
  msgTX.left_motor = leftspeed;
  msgTX.right_motor = rightspeed;
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C"
{
  int player_driver_init(DriverTable* table)
  {
    puts("LPuck driver initializing");
    LPuck_Register(table);
    puts("LPuck driver done");
    return(0);
  }
}


