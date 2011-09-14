#ifndef EPUCKSIM_H
#define EPUCKSIM_H
/*
#include <stdio.h>
#include <stdint.h>
#include <time.h>*/
#include <pthread.h>
#include "libplayerc++/playerc++.h"
#include "AudioHandler.h"
#include "EPuck.h"


/**
Interacts with a simulated e-puck robot using Player commands.

Upon initialisation this class will create a thread using POSIX which reads in the sensor data from the robot.
This allows multiple instances of EPuck to function in parallel and read in their sensor data with no effort from the programmer.

In order for this code to work your stage configuration needs to provide a simulation interface
which can then be shared between all the epucks in the simulation. Without this the code might not work, and the audio
and LED code definitely won't work. This can easily be done in the .world file, eg:<br>
<code><pre>
driver
(
  name "stage"
  provides ["6664:simulation:0"]
  plugin "stageplugin"

  # load the named file into the simulator
  worldfile "boids.world"
)
</pre></code>
@author Jennifer Owen
@see EPuck
@see EPuckReal
 */
class EPuckSim : public EPuck
{
public:

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

	EPuckSim(char* robotName);
	EPuckSim(int robotPort, char* robotName);
	EPuckSim(int robotPort, char* robotName, int simulationPort);
	~EPuckSim(void);

	/**
		Refreshes the robot's stored sensor values. Is automatically called by {@link #readSensorsThreaded readSensorsThreaded}
	 */
	void readSensors(void);

	/**
	 * Returns the elapsed time of the experiment as a double. In a simulation, simulated time should be used in preference of real time
	 * in case you ever want the simulation to be speeded up. Using real time on a speeded up simulation may mean that
	 * some processes are running much slower than others and will cause erratic and unexpected behaviour.
	 * @warning this function requires the development version of Stage to be installed for it to work.
	 * @warning this function casts from uint64_t to double, which may or may not cause troubles. If your OS is 32-bit it will be fine. It hasn't been tested on a 64-bit OS.
	 * @returns sim simulated time in milliseconds.
	 * NOTE that the stage simulator uses a time step of 100ms
	 * so the returned value of this function will be a factor of 100, this can be changed in the worldfile using
	 * the parameter "interval_sim".
	 * */
	double getTime(void);

	/**
	 * Gets the amount of volts currently being output by the epuck's battery.
	 * @returns voltage, the battery should normally be {@link EPuck#MAXIMUM_BATTERY_VOLTAGE}. It will be less if the battery is running low.
	 * */
	double getBatteryVolts(void);

	//==================== IR methods =========================================
	/**
		Gives the IR readings as an array of length returned by {@link #getNumberOfIRs getNumberOfIRs} class.
		@return The returned ranges for each IR sensor, these are normalised to be given in metres.
	 */
	double* getIRReadings(void);

	/**
		Gives the IR reading of a particular IR sensor.
		@param index The index of the sensor you want to measure. This will be a number between 0 and value returned by {@link #getNumberOfIRs getNumberOfIRs} - 1.
		@return The range returned by the specified IR sensor, normalised to be given in metres.
	 */
	double getIRReading(int index);

	/**
		Tells you how many IR sensors there are on the robot.
		@return The number of IR sensors as an int.
	 */
	int getNumberOfIRs(void);

	//==================== Blobfinder methods =====================================

	/**
	 * Returns the width, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
	 * @return the width, in pixels, of the camera image. If no blob is detected it will return -1.
	 * */
	int getCameraWidth(void);

	/**
	 * Returns the height, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
	 * @return the height, in pixels, of the camera image. If no blob is detected it will return -1.
	 * */
	int getCameraHeight(void);

	/**
	 * Returns the number of coloured blobs in the image
	 * @return the number of coloured blobs (of all colours)
	 * */
	int getNumberBlobs(void);

	/**
	 * Returns data about a specific blob, referenced by an ID number
	 * @return blob information, in the form of a Blob object
	 * @see Blob
	 * */
	Blob getBlob(int index);

	//==================== motor control methods ================================

	/**
		Sets the wheel speeds of the epuck's motors, requires a forward speed and a turnrate. Values are given in metres per second and radians per second.
		The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
		@param forward speed at which the robot moves forward in metres/sec
		@param turnrate speed at which the robot turns. Positive to turn left, negative to turn right. Value required in radians/sec.
		@see MAX_WHEEL_SPEED
	 */
	void setMotors(double forward, double turnrate);

	/**
		Sets the wheel speeds of the epuck's motors, this function is used to directly set the left and right wheel speed. Values are given in metres per second.
		The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
		@see MAX_WHEEL_SPEED
		@param left speed of the left wheel
		@param right speed of the right wheel
	 */
	void setDifferentialMotors(double left, double right);

	//==================== LED methods ==================================

	/**
	 * Sets all the robot LEDs into the ON state
	 * <br>
	 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
	 * */
	void setAllLEDsOn(void);

	/**
	 * Sets all the robot LEDs into the OFF state
	 * <br>
	 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
	 * */
	void setAllLEDsOff(void);

	/**
	 * Each time this function is called it will toggle between all the LEDs being on, and all the LEDs being off.
	 * */
	void toggleAllLEDs(void);

	/**
	 * Sets the specified LED into the specified state.
	 *  * <br>
	 * <b>WARNING: This function doesn't actually work in simulation. The entire robot can be either on or off.</b>
	 * @param index The index of the LED to change.
	 * @param state the state to set that LED to. 1 indicates on, anything else indicates off.
	 * */
	void setLED(int index, int state);

	/**
	 * Flashes the LEDs at the requested frequency.
	 * @param frequency the frequency in Hz at which the LEDs should flash.
	 * */
	void flashLEDs(double frequency);

	/**
	 * Stops the LEDs from flashing if they are already flashing.
	 * */
	void stopFlashLEDs(void);

	//======================== audio methods =================================

	/**
	 * Initialises the audio drivers so that we can use audio signals in stage.
	 * @returns success. 0 if audio handler is initialised, -1 if already initialised.
	 * */
	int initaliseAudio(void);

	/**
	 * Get this Epuck to play a tone of the desired frequency and duration.
	 * @param frequency frequency of tone to play in Hz
	 * @param duration duration of the tone in milliseconds
	 * @param volume the sound level (volume) to play the tone at. A number between 0 and 10. This does not go up to 11.
	 * @returns 0 if successful -1 if unsuccessful
	 */
	int playTone(int frequency, double duration, double volume);

	/**
	 * Listens for any sounds in the audio environment and stores them in the EPuck class until the user requests them.
	 * Tones of similar frequency are grouped together because a Fourier transform is performed on the signal from the microphones,
	 * the resulting information is combined in a way that is physically plausible (because this is a simulation after all...)
	 * and stored in the EPuck object until requested by the user.
	 * @returns numberOfTones the number of different tones the robot can hear.
	 * */
	int listenForTones(void);

	/**
	 * Will return the requested tone. The EPuck object stores a list of tones, their frequencies, volumes and directions wrt the epuck.
	 * This list of tones is updated when {@link EPuck#listenForTones} is called. This function allows you to request a tone from this array.
	 * @param index the index of the tone you wish to get from the EPuck object
	 * @returns tone the tone.
	 * */
	Tone getTone(int index);

#if DEBUGGING == 1
	void printLocation_TEST(void);
	void printTimes_TEST(void);
	void dumpAudio_TEST(void);
	void dumpToneData_TEST(AudioHandler::audio_message_t *store, size_t storesize);
#endif

private:
	/**
	 * Because constructors can't call other constructors, this is a common method that the overloaded constructors can call which will initialise the robot.
	 * @param port the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
	 * @param name the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
	 * @param simulationPort the port on which the simulation is running. Get this from the .cfg file of your simulation.
	 * */
	void initialise(int robotPort, char* robotName, int simulationPort);



	pthread_t readSensorsThread;
	void readSensorsThreaded(void);
	static void *startReadSensorThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<EPuckSim *>(obj)->readSensorsThreaded();
		return NULL;
	}

	pthread_t flashLEDsThread;
	void flashLEDsThreaded(void);
	static void *startFlashLEDsThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<EPuckSim *>(obj)->flashLEDsThreaded();
		return NULL;
	}



};




#endif
