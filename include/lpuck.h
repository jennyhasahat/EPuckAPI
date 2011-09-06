#ifndef LPUCK_H_
#define LPUCK_H_

// the number of 16 bit words that make up TX/RX
#define TXRX_SIZE	25

struct image_t
{
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_line;
    unsigned int size;
    void *buf;
};


struct cmd_t{
	int16_t set_motor:1;
	int16_t set_led:1;
	int16_t reset:1;
    int16_t cal_ir:1;
	int16_t cal_acc:1;
	int16_t read_ir:1;
	int16_t read_light:1;
	int16_t read_mic:1;
	int16_t read_acc:1;
	int16_t reseverd:7;
};
struct led_cmd_t{
	int16_t led0:1;
	int16_t led1:1;
	int16_t led2:1;
	int16_t led3:1;
	int16_t led4:1;
	int16_t led5:1;
	int16_t led6:1;
	int16_t led7:1;
	int16_t bodyled:1;
	int16_t frontled:1;
	int16_t reserved:6;
};

struct ir_pose_t{
	float x;
	float y;
	float th;
};


//note that the SPI works in full dulex mode, so better to keep the txbuf_t and rxbuf_t in the same size
//the dspic will receive the data in the same order but shift one bytes back
//for example, if we send (1,2,3,4,5,6,7,8) (1,2,3,4,5,6,7,8)
//then dspic will recieved (0,1,2,3,4,5,6,7) (8,1,2,3,4,5,6,7,8)
//so the last bytes(dummy) are not used for communication.
struct txbuf_t
{
    struct cmd_t cmd;		//first two bytes for commands
    int16_t left_motor;	//speed of left motor
    int16_t right_motor;	//speed of right motor
    struct led_cmd_t led_cmd;	//command for leds
    int16_t led_cycle;		// blinking rate of LEDS
    int16_t reserved[19];	//reserved, in order to makde the txbuf_t and rxbuf_t are in the same size, now 25 16bits words
    int16_t dummy;		// leave it empty
};
struct rxbuf_t
{
    int16_t ir[8];			// IR Ranges
    int16_t acc[3];			// Accelerometer x/y/z
    int16_t mic[3];			// microphones 1,2,3
    int16_t amb[8];			// Ambient IR
    int16_t tacl;			// steps made on l/r motors
    int16_t tacr;
    int16_t batt;			// battery level
};

#endif
