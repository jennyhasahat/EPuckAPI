#include "EPuckReal.h"


/*====================================================================
			CONSTRUCTOR/DESTRUCTOR
====================================================================*/


EPuckReal::EPuckReal(void)
{
	//initialise member variables
	allLEDsOn			= false;

	//make proxies
	try
	{
		epuck 		= new PlayerCc::PlayerClient("localhost");

		p2dProxy 	= new PlayerCc::Position2dProxy(epuck, 0);
		irProxy 	= new PlayerCc::IrProxy(epuck, 0);
		blobProxy 	= new PlayerCc::BlobfinderProxy(epuck, 0);
		powerProxy	= new PlayerCc::PowerProxy(epuck, 0);	//battery
	}
	catch (PlayerCc::PlayerError e)
	{
		std::cerr << e << std::endl;
		return;
	}

	//start threads
	pthread_create(&readSensorsThread, 0, EPuckReal::startReadSensorThread, this);

	//TODO seed RNG.
	//startTime = value read in from file
	//srand(startTime+time(NULL));
}


EPuckReal::~EPuckReal(void)
{
	//close threads
	pthread_cancel(readSensorsThread);
	stopFlashLEDs();	//stops the flashing LEDs and closes the thread


	//free the items in memory
	//this is probably unnecessary
	delete	p2dProxy;	//motors
	delete	irProxy;	//rangers
	delete	blobProxy;	//camera
	delete powerProxy; 	//battery
	delete	epuck;

	return;
}



/*====================================================================
			PUBLIC FUNCTIONS
====================================================================*/






/*
	READ SENSORS
*/


void EPuckReal::readSensors(void)
{	
	epuck->Read();
	return;
}


double EPuckReal::getTime(void)
{
	return time(NULL)+startTime;
}

//************INFRA-RED SENSORS*******************


double* EPuckReal::getIRReadings(void)
{
	int i;
	
	for(i=0; i<getNumberOfIRs(); i++)
	{
		irReadings[i] = irProxy->GetRange(i);
	}	
	
	return irReadings;
}


double EPuckReal::getIRReading(int index)
{
	return irProxy->GetRange(index);
}


int EPuckReal::getNumberOfIRs(void)
{
	return 8;
}


//************BLOBFINDER SENSORS*******************

/**
 * Returns the width, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
 * @return the width, in pixels, of the camera image. If no blob is detected it will return -1.
 * */
int EPuckReal::getCameraWidth(void)
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
int EPuckReal::getCameraHeight(void)
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
int EPuckReal::getNumberBlobs(void)
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
Blob EPuckReal::getBlob(int index)
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
The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
@param forward speed at which the robot moves forward in metres/sec
@param turnrate speed at which the robot turns. Positive to turn left, negative to turn right. Value required in radians/sec.
*/
void EPuckReal::setMotors(double forward, double turnrate)
{
	if(forward > EPuck::MAX_WHEEL_SPEED) forward = EPuck::MAX_WHEEL_SPEED;
	if(forward < (-1)*EPuck::MAX_WHEEL_SPEED) forward = (-1)*EPuck::MAX_WHEEL_SPEED;
	p2dProxy->SetSpeed(forward, turnrate);
	return;
}

/**
Sets the wheel speeds of the epuck's motors, this function is used to directly set the left and right wheel speed. Values are given in metres per second.
The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
@param left speed of the left wheel
@param right speed of the right wheel
*/
void EPuckReal::setDifferentialMotors(double left, double right)
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

/**
 * Sets all the robot LEDs into the ON state
 * <br>
 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
 * */
void EPuckReal::setAllLEDsOn(void)
{
	float red[]={1, 0, 0, 1};
	char colour[]="color";

	simProxy->SetProperty(name, colour, &red, sizeof(red));
	allLEDsOn = true;
	return;
}

/**
 * Sets all the robot LEDs into the OFF state
 * <br>
 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
 * */
void EPuckReal::setAllLEDsOff(void)
{
	float darkGreen[]={0.67, 0.88, 0.43, 1};
	char colour[]="colour";

	simProxy->SetProperty(name, colour, &darkGreen, sizeof(darkGreen));
	allLEDsOn = false;
	return;
}

/**
 * Each time this function is called it will toggle between all the LEDs being on, and all the LEDs being off.
 * */
void EPuckReal::toggleAllLEDs(void)
{
	if(allLEDsOn) setAllLEDsOff();
	else setAllLEDsOn();
	return;
}

/**
 * Sets the specified LED into the specified state.
 *  * <br>
 * <b>WARNING: This function doesn't actually work in simulation. The entire robot can be either on or off.</b>
 * @param index The index of the LED to change.
 * @param state the state to set that LED to. 1 indicates on, anything else indicates off.
 * */
void EPuckReal::setLED(int index, int state)
{
	if(state == 1) setAllLEDsOn();
	else setAllLEDsOff();
	return;
}

/**
 * Flashes the LEDs at the requested frequency.
 * @param frequency the frequency in Hz at which the LEDs should flash.
 * */
void EPuckReal::flashLEDs(double frequency)
{
	//user can also stop flashing LEDs with this function.
	if(frequency <= 0)
	{
		stopFlashLEDs();
		return;
	}

	LEDFlashFrequency = frequency;
	pthread_create(&flashLEDsThread, 0, EPuckReal::startFlashLEDsThread, this);
	return;
}

/**
 * Stops the LEDs from flashing if they are already flashing.
 * */
void EPuckReal::stopFlashLEDs(void)
{
//	if(LEDFlashFrequency != 0)
//	{
//		pthread_cancel(flashLEDsThread);
		LEDFlashFrequency = 0;
//	}
	return;
}

//******************************* AUDIO *************************************

/**
 * Initialises the audio drivers so that we can use audio signals in stage.
 * @returns success. 0 if audio handler is initialised, -1 if already initialised.
 * */
int EPuckReal::initaliseAudio(void)
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
 * @param volume the sound level (volume) to play the tone at. A number between 0 and 10. This does not go up to 11.
 * @returns 0 if successful -1 if unsuccessful
 */
int EPuckReal::playTone(int frequency, double duration, double volume)
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
 * @returns numberOfTones the number of different tones the robot can hear.
 * */
int EPuckReal::listenForTones(void)
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

/**
 * Will return the requested tone. The EPuck object stores a list of tones, their frequencies, volumes and directions wrt the epuck.
 * This list of tones is updated when {@link EPuck#listenForTones} is called. This function allows you to request a tone from this array.
 * @param index the index of the tone you wish to get from the EPuck object
 * @returns tone the tone.
 * */
EPuckReal::Tone EPuckReal::getTone(int index)
{
	if(index < numberOfTones && index > -1)
	{
		return toneArray[index];
	}
	else
	{
		printf("In EPuckReal::getTone, index %d does not exist.\n", index);
		EPuckReal::Tone t;
		t.bearing = 0;
		t.frequency = 0;
		t.volume = 0;
		return t;
	}
}

void EPuckReal::printLocation_TEST(void)
{
	double x, y, yaw;
	simProxy->GetPose2d(name, x, y, yaw);
	printf("%s is at location: (%f, %f) and yaw %f radians\n", name, x, y, yaw);

	return;
}

void EPuckReal::printTimes_TEST(void)
{
	printf("current time(NULL) is %f\n", (double)time(NULL));
	printf("current data time is %f\n", simProxy->GetDataTime());

	return;
}

void EPuckReal::dumpAudio_TEST(void)
{
	handler->dumpData_TEST();
	return;
}

void EPuckReal::dumpToneData_TEST(AudioHandler::audio_message_t *store, size_t storesize)
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

/*====================================================================
			PRIVATE FUNCTIONS
====================================================================*/


/**
Reads the robot's sensors but is threadable.
*/
void EPuckReal::readSensorsThreaded(void)
{
	printf("threaded\n");
	while(true)
	{
		epuck->Read();
		usleep(10);
	}
	pthread_exit(NULL);
	return;
}

/**
Reads the robot's sensors but is threadable.
*/
void EPuckReal::flashLEDsThreaded(void)
{
	double period = 1/LEDFlashFrequency; //flash period in seconds
	period = period*1000000; //now in micro-seconds
	period = period/2;	//will toggle LEDs each half period

	while(LEDFlashFrequency > 0)
	{
		toggleAllLEDs();
		usleep(period);
	}
	pthread_exit(NULL);
	return;
}






