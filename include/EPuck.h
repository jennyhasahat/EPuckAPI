#ifndef EPUCK_H
#define EPUCK_H

#include "libplayerc++/playerc++.h"
#include "DataStructures.h"

/**Debugging flag. If set to 1 the debugging functions are compiled and can be accessed.*/
#define DEBUGGING 0

/**
Interacts with a simulated e-puck robot using Player commands.
This is a base class for the more specific EPuckSim and EPuckReal classes, this class uses virtual functions so that polymorphism can be used.

An example of making an EPuck object to control a simulated EPuck:<br>
<code>EPuck robot = new EPuckSim("robot1");</code>

An example of making an EPuck object to control a real EPuck:<br>
<code>EPuck robot = new EPuckReal();</code>
@author Jennifer Owen
@see EPuckSim
@see EPuckReal
 */
class EPuck
{

public:


	//==========================================================
	//				CONSTANTS
	//==========================================================


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


	//==========================================================
	//				FUNCTIONS
	//==========================================================

public:

	/**
	 * Creates an instance of the EPuck class.
	 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
	 * using default simulation port 6664 and default robot port 6665.
	 * @param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
	 * @author Jennifer Owen
	 * *
	EPuck(char* robotName);

	**
	 * Creates an instance of the EPuck class.
	 * Calls {@link EPuck#EPuck(int robotPort, char* robotName, int simulationPort) EPuck(int robotPort, char* robotName, int simulationPort)}
	 * using default simulation port 6664.
	 * @param robotPort the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
	 * @param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
	 * @author Jennifer Owen
	 * *
	EPuck(int robotPort, char* robotName);

	**
	Creates and instance of the EPuck class.
	@param robotPort the number of the EPuck in the simulation. Eg 6665, 6666, 6667 etc.
	@param robotName the name of the robot model in the simulation eg robot1, robot2 etc. Maximum 64 chars.
	@param simulationPort the port on which the simulation is running. Get this from the .cfg file of your simulation.
	@author Jennifer Owen
	 *
	EPuck(int robotPort, char* robotName, int simulationPort);
	*/

	/**
	Epuck destructor. Closes all threads and stops the robot nicely (ish).
	 */
	virtual ~EPuck(void){};

	/**
	 * Allows two epucks to be compared to each other.
	 * */
	//virtual bool operator==(const EPuck& other);

	/**
	Refreshes the robot's stored sensor values. Is automatically called by {@link #readSensorsThreaded readSensorsThreaded}
	 */
	virtual void readSensors(void) = 0;

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
	virtual double getTime(void) = 0;

	/**
	 * Gets the amount of volts currently being output by the epuck's battery.
	 * @returns voltage, the battery should normally be {@link EPuck#MAXIMUM_BATTERY_VOLTAGE}. It will be less if the battery is running low.
	 * */
	virtual double getBatteryVolts(void) = 0;

	//==================== IR methods =========================================
	/**
	Gives the IR readings as an array of length returned by {@link #getNumberOfIRs getNumberOfIRs} class.
	@return The returned ranges for each IR sensor, these are normalised to be given in metres.
	 */
	virtual double* getIRReadings(void) = 0;

	/**
	Gives the IR reading of a particular IR sensor.
	@param index The index of the sensor you want to measure. This will be a number between 0 and value returned by {@link #getNumberOfIRs getNumberOfIRs} - 1.
	@return The range returned by the specified IR sensor, normalised to be given in metres.
	 */
	virtual double getIRReading(int index) = 0;

	/**
	Tells you how many IR sensors there are on the robot.
	@return The number of IR sensors as an int.
	 */
	virtual int getNumberOfIRs(void) = 0;

	//==================== Blobfinder methods =====================================

	/**
	 * Returns the width, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
	 * @return the width, in pixels, of the camera image. If no blob is detected it will return -1.
	 * */
	virtual int getCameraWidth(void) = 0;

	/**
	 * Returns the height, in pixels, of the camera image. This will only return a value when there is a blob in view (annoyingly).
	 * @return the height, in pixels, of the camera image. If no blob is detected it will return -1.
	 * */
	virtual int getCameraHeight(void) = 0;

	/**
	 * Returns the number of coloured blobs in the image
	 * @return the number of coloured blobs (of all colours)
	 * */
	virtual int getNumberBlobs(void) = 0;

	/**
	 * Returns data about a specific blob, referenced by an ID number
	 * @return blob information, in the form of a Blob object
	 * @see Blob
	 * */
	virtual Blob getBlob(int index) = 0;

	//==================== motor control methods ================================

	/**
	Sets the wheel speeds of the epuck's motors, requires a forward speed and a turnrate. Values are given in metres per second and radians per second.
	The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
	@param forward speed at which the robot moves forward in metres/sec
	@param turnrate speed at which the robot turns. Positive to turn left, negative to turn right. Value required in radians/sec.
	@see MAX_WHEEL_SPEED
	 */
	virtual void setMotors(double forward, double turnrate) = 0;

	/**
	Sets the wheel speeds of the epuck's motors, this function is used to directly set the left and right wheel speed. Values are given in metres per second.
	The maximum speed of the robots is 4cm/s so the wheels are limited to +/- 0.04 metres per second.
	@see MAX_WHEEL_SPEED
	@param left speed of the left wheel
	@param right speed of the right wheel
	 */
	virtual void setDifferentialMotors(double left, double right) = 0;

	//==================== LED methods ==================================

	/**
	 * Sets all the robot LEDs into the ON state
	 * <br>
	 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
	 * */
	virtual void setAllLEDsOn(void) = 0;

	/**
	 * Sets all the robot LEDs into the OFF state
	 * <br>
	 * @warning This function may not work in simulation due to a Player/Stage bug. Must use Player 3.0.2 or higher and Stage 3.2.3 or higher.
	 * */
	virtual void setAllLEDsOff(void) = 0;

	/**
	 * Each time this function is called it will toggle between all the LEDs being on, and all the LEDs being off.
	 * */
	virtual void toggleAllLEDs(void) = 0;

	/**
	 * Sets the specified LED into the specified state.
	 *  * <br>
	 * <b>WARNING: This function doesn't actually work in simulation. The entire robot can be either on or off.</b>
	 * @param index The index of the LED to change.
	 * @param state the state to set that LED to. 1 indicates on, anything else indicates off.
	 * */
	virtual void setLED(int index, int state) = 0;

	/**
	 * Flashes the LEDs at the requested frequency.
	 * @param frequency the frequency in Hz at which the LEDs should flash.
	 * */
	virtual void flashLEDs(double frequency) = 0;

	/**
	 * Stops the LEDs from flashing if they are already flashing.
	 * */
	virtual void stopFlashLEDs(void) = 0;

	//======================== audio methods =================================

	/**
	 * Initialises the audio drivers so that we can use audio signals in stage.
	 * @returns success. 0 if audio handler is initialised, -1 if already initialised.
	 * */
	virtual int initaliseAudio(void) = 0;

	/**
	 * Get this Epuck to play a tone of the desired frequency and duration.
	 * @param frequency frequency of tone to play in Hz
	 * @param duration duration of the tone in milliseconds
	 * @param volume the sound level (volume) to play the tone at. A number between 0 and 10. This does not go up to 11.
	 * @returns 0 if successful -1 if unsuccessful
	 */
	virtual int playTone(int frequency, double duration, double volume) = 0;

	/**
	 * Listens for any sounds in the audio environment and stores them in the EPuck class until the user requests them.
	 * Tones of similar frequency are grouped together because a Fourier transform is performed on the signal from the microphones,
	 * the resulting information is combined in a way that is physically plausible (because this is a simulation after all...)
	 * and stored in the EPuck object until requested by the user.
	 * @returns numberOfTones the number of different tones the robot can hear.
	 * */
	virtual int listenForTones(void) = 0;

	/**
	 * Will return the requested tone. The EPuck object stores a list of tones, their frequencies, volumes and directions wrt the epuck.
	 * This list of tones is updated when {@link EPuck#listenForTones} is called. This function allows you to request a tone from this array.
	 * @param index the index of the tone you wish to get from the EPuck object
	 * @returns tone the tone.
	 * */
	virtual Tone getTone(int index) = 0;


};




#endif
