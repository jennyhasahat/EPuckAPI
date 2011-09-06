#ifndef EPUCK_H
#define EPUCK_H
/*
#include <stdio.h>
#include <stdint.h>
#include <time.h>*/
#include <pthread.h>
#include "libplayerc++/playerc++.h"
#include "AudioHandler.h"

/**Debugging flag. If set to 1 the debugging functions are compiled and can be accessed.*/
#define DEBUGGING 1

/**
Interacts with a simulated e-puck robot using Player commands.
This is a base class for the more specific SimulatedRobot and RealRobot classes, this class uses virtual functions so that polymorphism can be used.
<br>
Upon initialisation this class will create a thread using POSIX which reads in the sensor data from the robot.
This allows multiple instances of EPuck to function in parallel and read in their sensor data with no effort from the programmer.
Any interaction with the robot must be done through this class to localise the use of Player syntax.
@author Jennifer Owen
 */
class EPuckReal
{
public:

	/**
	 * A substitute for the player_blobfinder_blob_t structure. This one has all the variables in nice units (mostly ints),
	 * instead of sucky ones (mostly uint32_t). Plus it doesn't require and Player headers to be included outside of this API.
	 * Data is stored as member variables in the class, they are all public so can be accessed using the dot operator (.).
	 * As in blobobject.variableName eg blobobject.x or blobobject.area
	 * */
	class Blob
	{
	public:
		/**Index reference that the blobfinder proxy gave to this blob*/
		int	id;
		/**Colour of the blob in hex. Format is 0xaaRRGGBB, aa being the alpha component of the colour*/
		//TODO fix colours so that it works with the stuff you wrote for stage.
		uint32_t colour;
		/**area of the blob*/
		int area;
		/**Centre of the blob, coordinates referenced by top left corner of camera image*/
		int x;
		/**Centre of the blob, coordinates referenced by top left corner of camera image*/
		int y;
		/**Left edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int left;
		/**right edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int right;
		/**top edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int top;
		/**bottom edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int bottom;
	};

	/**
	 * Stores information about the tones in a single frequency band that can be heard by the robot.
	 * This is the information that the robot will be able to detect and is all robot-centric.
	 * @see EPuck#getTone
	 * @see EPuck#listenForTones
	 * */
	class Tone
	{
	public:
		/**The lower bound on the frequency range this tone could be*/
		double frequency;
		/**The volume of the tone. This is some arbitrary number without a real measurement, but they are consistent with each other so can be
		 * compared to other tones and volumes*/
		double volume;
		/**The bearing of the sound source with respect to the EPuck. If it is directly in front of the EPuck this will be 0,
		 * bearings are then measured in DEGREES anticlockwise from the robot's front.*/
		int bearing;
	};


	//constants

	/**The maximum wheel speed that the epuck can turn its wheels at.*/
	static const double MAX_WHEEL_SPEED = 0.041;
	/**The impedance of the robot's microphone*/
	static const int IMPEDANCE_OF_MIC_OHMS = 300;
	/**The impedance of the robot's speaker*/
	static const int IMPEDANCE_OF_SPEAKER_OHMS = 8;
	/**How much voltage is supplied to the epuck*/
	static const double MAXIMUM_BATTERY_VOLTAGE = 3.6;
	/**How much voltage is supplied to the epuck (minimum)*/
	static const double MINIMUM_BATTERY_VOLTAGE = 0;


	//member variables

	/**The Player/Stage port that this robot uses to get simulation information*/
	int port;
	/**The name given to this robot in the player/Stage configuration file and world file.*/
	char name[32];

protected:

	//player object member variables
	PlayerCc::PlayerClient		*epuck;
	PlayerCc::PlayerClient		*simulation;

	PlayerCc::Position2dProxy	*p2dProxy;		//motors
	PlayerCc::SonarProxy		*sonarProxy;	//rangers
	PlayerCc::BlobfinderProxy	*blobProxy;		//camera
	PlayerCc::SimulationProxy	*simProxy;		//leds

	/**array containing the IR readings from the EPuck*/
	double irReadings[8];
	//LED stuff
	bool allLEDsOn;
	double LEDFlashFrequency;

	//audio stuff
	AudioHandler *handler;
	bool audioInitialised;
	Tone *toneArray;
	int numberOfTones;

	//robot also supports power, aio and blinkenlight proxies
	//as far as I can tell, stage does not support these

public:

	EPuckReal(char* robotName);
	EPuckReal(int robotPort, char* robotName);
	EPuckReal(int robotPort, char* robotName, int simulationPort);
	~EPuckReal(void);

	void readSensors(void);
	double getTime(void);

	// IR methods
	double* getIRReadings(void);
	double getIRReading(int index);
	int getNumberOfIRs(void);

	// Blobfinder methods
	int getCameraWidth(void);
	int getCameraHeight(void);
	int getNumberBlobs(void);
	Blob getBlob(int index);

	// motor control methods
	void setMotors(double forward, double turnrate);
	void setDifferentialMotors(double left, double right);

	// LED methods
	void setAllLEDsOn(void);
	void setAllLEDsOff(void);
	void toggleAllLEDs(void);
	void setLED(int index, int state);
	void flashLEDs(double frequency);
	void stopFlashLEDs(void);

	//audio methods
	int initaliseAudio(void);
	int playTone(int frequency, double duration, double volume);
	int listenForTones(void);
	Tone getTone(int index);

#if DEBUGGING == 1
	void printLocation_TEST(void);
	void printTimes_TEST(void);
	void dumpAudio_TEST(void);
	void dumpToneData_TEST(AudioHandler::audio_message_t *store, size_t storesize);
#endif

protected:

	pthread_t readSensorsThread;
	void readSensorsThreaded(void);
	static void *startReadSensorThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<EPuckReal *>(obj)->readSensorsThreaded();
		return NULL;
	}

	pthread_t flashLEDsThread;
	void flashLEDsThreaded(void);
	static void *startFlashLEDsThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<EPuckReal *>(obj)->flashLEDsThreaded();
		return NULL;
	}


private:
	void initialise(int robotPort, char* robotName, int simulationPort);





};




#endif
