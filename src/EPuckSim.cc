#include "EPuckSim.h"


/*====================================================================
			CONSTRUCTOR/DESTRUCTOR
====================================================================*/

/**
 * Creates an instance of the EPuck class.
 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
 * using default simulation port 6664 and default robot port 6665.
 * @param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
 * @author Jennifer Owen
 * */
EPuckSim::EPuckSim(char* robotName)
{
	initialise(6665, robotName, 6664);
}

/**
 * Creates an instance of the EPuck class.
 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
 * using default simulation port 6664.
 * @param robotPort the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
 * @param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
 * @author Jennifer Owen
 * */
EPuckSim::EPuckSim(char* robotName, int robotPort)
{
	initialise(robotPort, robotName, 6664);
}

/**
Creates and instance of the EPuck class.
@param robotPort the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
@param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
@param simulationPort the port on which the simulation is running. Get this from the .cfg file of your simulation.
@author Jennifer Owen
*/
EPuckSim::EPuckSim(char* robotName, int robotPort, int simulationPort)
{	
	initialise(robotPort, robotName, simulationPort);
	return;
}



/**
Epuck destructor. Closes threads and stops the robot nicely (ish).
*/
EPuckSim::~EPuckSim(void)
{
	//close threads
	readSensorsThread.interrupt();
	readSensorsThread.join();

	stopFlashLEDs();	//stops the flashing LEDs and closes the thread


	//free the items in memory
	//this is probably unnecessary
	delete	p2dProxy;		//motors
	delete	rangerProxy;	//rangers
	delete	blobProxy;		//camera
	delete	simProxy;		//leds
	delete	epuck;
	delete	simulation;

	if(audioInitialised) delete[] toneArray;

	return;
}


bool EPuckSim::operator==(const EPuckSim& other)
{
	int comparison = strcmp(other.name, name);
	if(comparison == 0) return true;
	else return false;
}


/*====================================================================
			PUBLIC FUNCTIONS
====================================================================*/





/*
	READ SENSORS
*/

void EPuckSim::readSensors(void)
{	
	epuck->Read();
	return;
}

double EPuckSim::getTime(void)
{
	uint64_t data;
	double time;
	char flag[] = "time";

	simProxy->GetProperty(name, flag, &data, sizeof(data));
//printf("read value %f\n", (double)data);
	time = (double)data;
	time = time / 1000000;

	return time;
}

double EPuckSim::getBatteryVolts(void)
{
	return EPuck::MAXIMUM_BATTERY_VOLTAGE;
}

void EPuckSim::getPosition(double& x, double& y, double& yaw)
{
	simProxy->GetPose2d(name, x, y, yaw);
	return;
}

//************INFRA-RED SENSORS*******************


double* EPuckSim::getIRReadings(void)
{
	int i;
	
	for(i=0; i<getNumberOfIRs(); i++)
	{
		irReadings[i] = rangerProxy->GetRange(i);
	}	
	
	return irReadings;
}


double EPuckSim::getIRReading(int index)
{
	return rangerProxy->GetRange(index);
}

/**
Tells you how many IR sensors there are on the robot.
@return The number of IR sensors as an int.
*/
int EPuckSim::getNumberOfIRs(void)
{
	return 8;
}


//************BLOBFINDER SENSORS*******************


int EPuckSim::getCameraWidth(void)
{
	uint32_t w;
	int width;
	w = blobProxy->GetWidth();
	width = (int)w;

	if(width <= 0) return -1;
	else return width;
}


int EPuckSim::getCameraHeight(void)
{
	uint32_t h;
	int height;
	h = blobProxy->GetHeight();
	height = (int)h;

	if(height <= 0) return -1;
	else return height;
}


int EPuckSim::getNumberBlobs(void)
{
	uint32_t noBlobs;
	noBlobs = blobProxy->GetCount();
	return (int)noBlobs;
}


Blob EPuckSim::getBlob(int index)
{
	player_blobfinder_blob_t oldBlob;
	Blob newBlob;

	oldBlob = blobProxy->GetBlob(index);
	newBlob.id = (int)oldBlob.id;
	newBlob.colour = oldBlob.color;
	newBlob.area = (int)oldBlob.area;
	newBlob.x = (int)oldBlob.x;
	newBlob.y = (int)oldBlob.y;
	newBlob.left = (int)oldBlob.left;
	newBlob.right = (int)oldBlob.right;
	newBlob.top = (int)oldBlob.top;
	newBlob.bottom = (int)oldBlob.bottom;

	return newBlob;
}


/*
	USE ACTUATORS
*/

//*************************** MOTORS *****************************


void EPuckSim::setMotors(double forward, double turnrate)
{
	if(forward > EPuck::MAX_WHEEL_SPEED) forward = EPuck::MAX_WHEEL_SPEED;
	if(forward < (-1)*EPuck::MAX_WHEEL_SPEED) forward = (-1)*EPuck::MAX_WHEEL_SPEED;
	p2dProxy->SetSpeed(forward, turnrate);
	return;
}


void EPuckSim::setDifferentialMotors(double left, double right)
{
	/* conversion of differential drive values to 
	   forward and turn speeds from 
	   http://www.physicsforums.com/showthread.php?t=263149 */
	
	//the inside wheel will have to turn at x/R radians per second while outside wheel turns at (x+ yL)/R radians per second.
	// L is distance between wheels, R is radius of wheels, x is forward speed y is turnrate in rads/sec.
	
	//const double radius = 40*0.001; //40 millimetres
	const double separation = 52*0.001; //52 millimetres
	double newspeed, newturnrate;
	
	//limit wheel speeds to+/- maximum
	if(left > EPuck::MAX_WHEEL_SPEED) left = EPuck::MAX_WHEEL_SPEED;
	if(left < (-1)*EPuck::MAX_WHEEL_SPEED) left = (-1)*EPuck::MAX_WHEEL_SPEED;
	if(right > EPuck::MAX_WHEEL_SPEED) right = EPuck::MAX_WHEEL_SPEED;
	if(right < (-1)*EPuck::MAX_WHEEL_SPEED) right = (-1)*EPuck::MAX_WHEEL_SPEED;

	/*inside wheel should turn at newspeed/radius radians per sec.
	  inner(rads) * radius = newspeed
	  we know wheel turn rate in metres/sec. Must convert to rads/sec.
	  no wheel turns per sec is = inner(m)/2piRadius
	  inner(rads) = no wheel turns * 2pi
	  inner(rads) = inner(m)/Radius
	  inner(m)/Radius * radius = newspeed
	*/
	
	/* outer wheel turns at (newspeed+ newturnrate(rads)*separation)/Radius rads per second
	   from before: outer(rads) = outer(m)/Radius
	   outer(rads) = (newspeed+ newturnrate(rads)*separation)/Radius
	   outer(m)/Radius = (newspeed+ newturnrate(rads)*separation)/Radius
	   outer(m) = newspeed+ newturnrate(rads)*separation
	   newturnrate(rads) = (outer(m) - newspeed)/separation   
	*/
	
	//which is the outer wheel and which is the inner?
	//outer wheel moves further so will be faster.
	if(left > right) //if left is outer wheel
	{
		//newspeed is inner wheel speed
		newspeed = right;
		//newturnrate = (outer(m) - newspeed)/separation
		newturnrate = (left - newspeed)/separation;
		newturnrate *= -1;
	}
	else if(right > left) //right is outer wheel
	{
		//newspeed is inner wheel speed
		newspeed = left;
		//newturnrate = (outer(m) - newspeed)/separation
		newturnrate = (right - newspeed)/separation;
	}
	else //going straight ahead
	{
		newspeed = left;
		newturnrate = 0;
	}
	
	//p2dProxy->SetSpeed(newspeed, newturnrate);
	setMotors(newspeed, newturnrate);
	return;
}

//******************************* LED FLASHING *************************************


void EPuckSim::setAllLEDsOn(void)
{
	float red[]={1, 0, 0, 1};
	char colour[]="color";

	simProxy->SetProperty(name, colour, &red, sizeof(red));
	allLEDsOn = true;
	return;
}


void EPuckSim::setAllLEDsOff(void)
{
	float darkGreen[]={0.67, 0.88, 0.43, 1};
	char colour[]="color";

	simProxy->SetProperty(name, colour, &darkGreen, sizeof(darkGreen));
	allLEDsOn = false;
	return;
}


void EPuckSim::toggleAllLEDs(void)
{
	if(allLEDsOn) setAllLEDsOff();
	else setAllLEDsOn();
	return;
}


void EPuckSim::setLED(int index, int state)
{
	if(state == 1) setAllLEDsOn();
	else setAllLEDsOff();
	return;
}


void EPuckSim::flashLEDs(double frequency)
{
	//user can also stop flashing LEDs with this function.
	if(frequency <= 0)
	{
		stopFlashLEDs();
		return;
	}

	flashLEDsThread = boost::thread(&EPuckSim::flashLEDsThreaded, this, frequency);
	//pthread_create(&flashLEDsThread, 0, EPuckSim::startFlashLEDsThread, this);
	return;
}


void EPuckSim::stopFlashLEDs(void)
{
	flashLEDsThread.interrupt();
	flashLEDsThread.join();

	return;
}

//******************************* AUDIO *************************************


int EPuckSim::initaliseAudio(void)
{
	if(!audioInitialised)
	{
		handler = AudioHandler::GetAudioHandler(simulation, simProxy, name);
		audioInitialised = TRUE;
		return 0;
	}

	return -1;
}


int EPuckSim::playTone(int frequency, double duration, double volume)
{
	if(audioInitialised)
	{
		handler->playTone(frequency, duration, volume, name);
		return 0;
	}

	printf("Unsuccessful epuck %s playTone() request. Audio not initialised.\n", name);
	return -1;
}


int EPuckSim::listenForTones(void)
{
	int i;
	AudioHandler::audio_message_t *message;

	if(audioInitialised)
	{
		//need to remember how many tones were allocated last time and free that memory
		delete[] toneArray;

		//reserve space for new tone data
		numberOfTones 	= handler->getNumberOfTones();
		toneArray 		= new Tone[numberOfTones];
		message 		= new AudioHandler::audio_message_t[numberOfTones];
		handler->getTones(name, message, sizeof(AudioHandler::audio_message_t)*numberOfTones);

		for(i=0; i<numberOfTones; i++)
		{
			toneArray[i].volume 	= message[i].volume;
			toneArray[i].bearing 	= message[i].direction;
			toneArray[i].frequency 	= message[i].frequency;
		}

		return numberOfTones;
	}

	printf("Unsuccessful epuck %s listenToTones() request. Audio not initialised.\n", name);
	return -1;
}


Tone EPuckSim::getTone(int index)
{
	if(index < numberOfTones && index > -1)
	{
		return toneArray[index];
	}
	else
	{
		printf("In EPuckSim::getTone, index %d does not exist.\n", index);
		Tone t;
		t.bearing = 0;
		t.frequency = 0;
		t.volume = 0;
		return t;
	}
}

#if DEBUGGING == 1
	void EPuckSim::printLocation_TEST(void)
	{
		double x, y, yaw;
		simProxy->GetPose2d(name, x, y, yaw);
		printf("%s is at location: (%f, %f) and yaw %f radians\n", name, x, y, yaw);

		return;
	}

	void EPuckSim::printTimes_TEST(void)
	{
		printf("current time(NULL) is %f\n", (double)time(NULL));
		printf("current data time is %f\n", simProxy->GetDataTime());

		return;
	}

	void EPuckSim::dumpAudio_TEST(void)
	{
		handler->dumpData_TEST();
		return;
	}

	void EPuckSim::dumpToneData_TEST(AudioHandler::audio_message_t *store, size_t storesize)
	{
		int i;
		int numberAllocatedSlots = storesize/sizeof(AudioHandler::audio_message_t);

		printf("Dumping tone data as recieved from AudioHandler:\n");
		for(i=0; i<numberAllocatedSlots; i++)
		{
			printf("\tbin number %d\n", i);
			printf("\tfrequency %f\n\tvolume %f\n\tdirection %d\n", store[i].frequency, store[i].volume, store[i].direction);
		}
	}
#endif

/*====================================================================
			PRIVATE FUNCTIONS
====================================================================*/



void EPuckSim::initialise(int robotPort, char* robotName, int simulationPort)
{
	//initialise member variables
	strcpy(name, robotName);
	port 				= robotPort;
	allLEDsOn			= false;
	audioInitialised 	= false;
	toneArray 			= NULL;

	try
	{
		epuck 		= new PlayerCc::PlayerClient("localhost", port);
		simulation 	= new PlayerCc::PlayerClient("localhost", simulationPort);

		p2dProxy 	= new PlayerCc::Position2dProxy(epuck, 0);
		rangerProxy 	= new PlayerCc::RangerProxy(epuck, 0);
		blobProxy 	= new PlayerCc::BlobfinderProxy(epuck, 0);
		simProxy 	= new PlayerCc::SimulationProxy(simulation, 0);
	}
	catch (PlayerCc::PlayerError e)
	{
		std::cerr << e << std::endl;
		return;
	}

	readSensorsThread = boost::thread(&EPuckSim::readSensorsThreaded, this);
//	pthread_create(&readSensorsThread, 0, EPuckSim::startReadSensorThread, this);
}


void EPuckSim::readSensorsThreaded(void)
{
	printf("%s is threaded\n", name);
	boost::posix_time::milliseconds wait(10);

	while(true)
	{
		epuck->Read();
		boost::this_thread::sleep(wait);
	}

	return;
}


void EPuckSim::flashLEDsThreaded(int ledFlashFrequency)
{
	double period = 1/ledFlashFrequency; //flash period in seconds
	period = period*1000; //now in milliseconds
	period = period/2;	//will toggle LEDs each half period
	boost::posix_time::milliseconds wait(period);

	while(true)
	{
		toggleAllLEDs();
		boost::this_thread::sleep(wait);
	}

	return;
}






