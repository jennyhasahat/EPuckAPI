#include "../include/EPuck.h"

/*====================================================================
			CONSTRUCTOR/DESTRUCTOR
====================================================================*/

/**
 * Creates an instance of the EPuck class.
 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
 * using default simulation port 6664 and default robot port 6665.
 * @param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
 * */
EPuck::EPuck(char* robotName)
{
	initialise(6665, robotName, 6664);
}

/**
 * Creates an instance of the EPuck class.
 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
 * using default simulation port 6664.
 * @param port the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
 * @param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
 * */
EPuck::EPuck(int robotPort, char* robotName)
{
	initialise(robotPort, robotName, 6664);
}

/**
Creates and instance of the EPuck class.
@param port the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
@param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
@param simulationPort the port on which the simulation is running. Get this from the .cfg file of your simulation.
*/
EPuck::EPuck(int robotPort, char* robotName, int simulationPort)
{	
	initialise(robotPort, robotName, simulationPort);
	return;
}



/**
Epuck destructor. Closes threads and stops the robot nicely (ish).
*/
EPuck::~EPuck(void)
{
#if THREADED
	pthread_detach(readSensorsThread);
#endif

	//free the items in memory
	//this is probably unnecessary
	delete	p2dProxy;		//motors
	delete	sonarProxy;	//rangers
	delete	blobProxy;		//camera
	delete	simProxy;		//leds
	delete	epuck;
	delete	simulation;

	if(audioInitialised) delete[] toneArray;

	return;
}



/*====================================================================
			PUBLIC FUNCTIONS
====================================================================*/






/*
	READ SENSORS
*/

/**
Refreshes the robot's stored sensor values.
*/
void EPuck::readSensors(void)
{	
	epuck->Read();
	return;
}

/**
 * Returns the simulated time of the experiment as a double. Simulated time should be used in preference of real time
 * in case you ever want the simulation to be speeded up. Using real time on a speeded up simulation may mean that
 * some processes are running much slower than others and will cause erratic and unexpected behaviour.
 * @warning this function requires the development version of Stage to be installed for it to work.
 * @warning this function casts from uint64_t to double, which may or may not cause troubles. If your OS is 32-bit it will be fine. It hasn't been tested on a 64-bit OS.
 * @returns sim simulated time in milliseconds. NOTE that the stage simulator uses a time step of 100ms so the returned value of this function will be a factor of 100.
 * */
double EPuck::getSimulationTime(void)
{
	uint64_t data;
	double time;
	char flag[] = "simtime";

	simProxy->GetProperty(name, flag, &data, sizeof(data));

	time = (double)data;
	time = time / 1000;

	return time;
}

//************INFRA-RED SENSORS*******************

/**
Gives the IR readings as an array of length returned by {@link #getNumberOfIRs(void) getNumberOfIRs} class.
@return The returned ranges for each IR sensor, these are normalised to be given in metres.
*/
double* EPuck::getIRReadings(void)
{
	int i;
	
	for(i=0; i<getNumberOfIRs(); i++)
	{
		irReadings[i] = sonarProxy->GetScan(i);
	}	
	
	return irReadings;
}

/**
Gives the IR reading of a particular IR sensor.
@param index The index of the sensor you want to measure. This will be a number between 0 and value returned by {@link #getNumberOfIRs(void) getNumberOfIRs} - 1.
@return The range returned by the specified IR sensor, normalised to be given in metres.
*/
double EPuck::getIRReading(int index)
{
	return sonarProxy->GetScan(index);
}

/**
Tells you how many IR sensors there are on the robot.
@return The number of IR sensors as an int.
*/
int EPuck::getNumberOfIRs(void)
{
	return 8;
}


//************BLOBFINDER SENSORS*******************

/**
 * Returns the width, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
 * @return the width, in pixels, of the camera image. If no blob is detected it will return -1.
 * */
int EPuck::getCameraWidth(void)
{
	uint32_t w;
	int width;
	w = blobProxy->GetWidth();
	width = (int)w;

	if(width <= 0) return -1;
	else return width;
}

/**
 * Returns the height, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
 * @return the height, in pixels, of the camera image. If no blob is detected it will return -1.
 * */
int EPuck::getCameraHeight(void)
{
	uint32_t h;
	int height;
	h = blobProxy->GetHeight();
	height = (int)h;

	if(height <= 0) return -1;
	else return height;
}

/**
 * Returns the number of coloured blobs in the image
 * @return the number of coloured blobs (of all colours)
 * */
int EPuck::getNumberBlobs(void)
{
	uint32_t noBlobs;
	noBlobs = blobProxy->GetCount();
	return (int)noBlobs;
}

/**
 * Returns data about a specific blob, referenced by an ID number
 * @return blob information, in the form of a Blob object
 * @see  EPuck.h#Blob Blob
 * */
EPuck::Blob EPuck::getBlob(int index)
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

/**
Sets the wheel speeds of the epuck's motors, requires a forward speed and a turnrate. Values are given in metres per second and radians per second.
@param forward speed at which the robot moves forward in metres/sec
@param turnrate speed at which the robot turns. Positive to turn left, negative to turn right. Value required in radians/sec.
*/
void EPuck::setMotors(double forward, double turnrate)
{
	p2dProxy->SetSpeed(forward, turnrate);
	return;
}

/**
Sets the wheel speeds of the epuck's motors, this function is used to directly set the left and right wheel speed. Values are given in metres per second.
@param left speed of the left wheel
@param right speed of the right wheel
*/
void EPuck::setDifferentialMotors(double left, double right)
{
	/* conversion of differential drive values to 
	   forward and turn speeds from 
	   http://www.physicsforums.com/showthread.php?t=263149 */
	
	//the inside wheel will have to turn at x/R radians per second while outside wheel turns at (x+ yL)/R radians per second.
	// L is distance between wheels, R is radius of wheels, x is forward speed y is turnrate in rads/sec.
	
	//const double radius = 40*0.001; //40 millimetres
	const double separation = 52*0.001; //52 millimetres
	double newspeed, newturnrate;
	
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

/**
 * Sets all the robot LEDs into the ON state
 * <br>
 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
 * */
void EPuck::setAllLEDSOn(void)
{
	uint32_t red = 0xffff0000;
	char colour[]="MP_COLOR";

	simProxy->SetProperty(name, colour, &red, sizeof(red));

	return;
}

/**
 * Sets all the robot LEDs into the OFF state
 * <br>
 * <b>WARNING: This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.</b>
 * */
void EPuck::setAllLEDSOff(void)
{
	uint32_t darkGreen = 0xff006400;
	char colour[]="MP_COLOR";

	simProxy->SetProperty(name, colour, &darkGreen, sizeof(darkGreen));
	return;
}

/**
 * Sets the specified LED into the specified state.
 *  * <br>
 * <b>WARNING: This function doesn't actually work in simulation. The entire robot can be either on or off.</b>
 * @param index The index of the LED to change.
 * @param the state to set that LED to. 1 indicates on, anything else indicates off.
 * */
void EPuck::setLED(int index, int state)
{
	if(state == 1) setAllLEDSOn();
	else setAllLEDSOff();
	return;
}

//******************************* AUDIO *************************************

/**
 * Initialises the audio drivers so that we can use audio signals in stage.
 * @returns success. 0 if audio handler is initialised, -1 if already initialised.
 * */
int EPuck::initaliseAudio(void)
{
	if(!audioInitialised)
	{
		handler = AudioHandler::GetAudioHandler(simulation, simProxy, name);
		audioInitialised = TRUE;
		return 0;
	}

	return -1;
}

/**
 * Get this Epuck to play a tone of the desired frequency and duration.
 * @param frequency frequency of tone to play in Hz
 * @param duration duration of the tone in milliseconds
 * @param volume the sound level (volume) to play the tone at
 * @returns 0 if successful -1 if unsuccessful
 */
int EPuck::playTone(int frequency, double duration, double volume)
{
	if(audioInitialised)
	{
		handler->playTone(frequency, duration, volume, name);
		return 0;
	}

	printf("Unsuccessful epuck %s playTone() request. Audio not initialised.\n", name);
	return -1;
}

/**
 * Listens for any sounds in the audio environment and stores them in the EPuck class until the user requests them.
 * Tones of similar frequency are grouped together because a Fourier transform is performed on the signal from the microphones,
 * the resulting information is combined in a way that is physically plausible (because this is a simulation after all...)
 * and stored in the EPuck object until requested by the user.
 * @returns numberoftones the number of different tones the robot can hear.
 * */
int EPuck::listenForTones(void)
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
		handler->getTones(name, message, numberOfTones);

		for(i=0; i<numberOfTones; i++)
		{
			toneArray[i].volume 	= message->volume;
			toneArray[i].bearing 	= message->direction;
			toneArray[i].frequency 	= message->frequency;
		}

		return 0;
	}

	printf("Unsuccessful epuck %s listenToTones() request. Audio not initialised.\n", name);
	return -1;
}

/**
 * Will return the requested
 * If listenForTones() has
 * */
EPuck::Tone EPuck::getTone(int index)
{
	if(index < numberOfTones || index > -1)
	{
		return toneArray[index];
	}
	else
	{
		printf("In EPuck::getTone, index %d does not exist.\n", index);
	}
	//return NULL;
}


void EPuck::printTimes_TEST(void)
{
	printf("current time(NULL) is %f\n", (double)time(NULL));
	printf("current data time is %f\n", simProxy->GetDataTime());

	return;
}

void EPuck::dumpAudio_TEST(void)
{
	handler->dumpData_TEST();
	return;
}

/*====================================================================
			PRIVATE FUNCTIONS
====================================================================*/


/**
 * Because constructors can't call other constructors, this is a common method that the overloaded constructors can call which will initialise the robot.
 * @param port the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
 * @param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
 * @param simulationPort the port on which the simulation is running. Get this from the .cfg file of your simulation.
 * */
void EPuck::initialise(int robotPort, char* robotName, int simulationPort)
{
	//initialise member variables
	strcpy(name, robotName);
	port 				= robotPort;
	audioInitialised 	= false;
	toneArray 			= NULL;

	try
	{
		epuck 		= new PlayerCc::PlayerClient("localhost", port);
		simulation 	= new PlayerCc::PlayerClient("localhost", simulationPort);

		p2dProxy 	= new PlayerCc::Position2dProxy(epuck, 0);
		sonarProxy 	= new PlayerCc::SonarProxy(epuck, 0);
		blobProxy 	= new PlayerCc::BlobfinderProxy(epuck, 0);
		simProxy 	= new PlayerCc::SimulationProxy(simulation, 0);
	}
	catch (PlayerCc::PlayerError e)
	{
		std::cerr << e << std::endl;
		return;
	}

#if THREADED
	pthread_create(&readSensorsThread, 0, EPuck::startReadSensorThread, this);
#endif

}


#if THREADED
/**
Reads the robot's sensors but is threadable.
*/
void EPuck::readSensorsThreaded(void)
{
	printf("%s is threaded\n", name);
	while(true)
	{
		epuck->Read();
		usleep(10);
	}
	pthread_exit(NULL);
	return;
}
#endif






