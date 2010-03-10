#include "../include/EPuck.h"

/*====================================================================
			CONSTRUCTOR/DESTRUCTOR
====================================================================*/

/**
Interacts with a simulated e-puck robot using Player commands.
This is a base class for the more specific SimulatedRobot and RealRobot classes, this class uses virtual functions so that polymorphism can be used.
<br>
Before each call to a sensor or actuator the sensor readings are updated. The usual player thing to do is have the sensors update every fraction of a second regardless of whether a reading is actually needed. 
The EPuck object does this because using threads makes the player/stage simulation very choppy. Threads may work in the normal robot but it's doubtful.
@param port the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
@param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
*/
EPuck::EPuck(int robotPort, char* robotName)
{	
	port = robotPort;
	strncpy(name, robotName, strlen(robotName+1));
	
	try
	{
		epuck = new PlayerCc::PlayerClient("localhost", port);
		p2dProxy = new PlayerCc::Position2dProxy(epuck, 0);
		sonarProxy = new PlayerCc::SonarProxy(epuck, 0);
		blobProxy = new PlayerCc::BlobfinderProxy(epuck, 0);
	}
	catch (PlayerCc::PlayerError e)
	{
		std::cerr << e << std::endl;
		return;
	}
	
#if THREADED
	pthread_create(&readSensorsThread, 0, EPuck::startReadSensorThread, this);
#endif
				
	printf("%s created\n", robotName);
	
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
Blob EPuck::getBlob(int index)
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

/*====================================================================
			PRIVATE FUNCTIONS
====================================================================*/


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




