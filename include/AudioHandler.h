/**
 * AudioHandler stores the audio environment data in the simulation.
 * It provides interfaces for storing audio information and for accessing audio information.
 * A number of simplifications about sound have been made in this class for the purpose of creating a simulation:<p>
 * <ul>
 * <li>Electrical watts and sound intensity watts are the same (it's all joules per second after all).</li>
 * <li>there is no interference between tones of different frequencies</li>
 * <li>All frequencies decay over distance at the same rate</li>
 * <li>The environment has no reverberations to confuse the direction and level of a sound</li>
 * <li>Robots don't move whilst making a sound</li>
 * <li>Obstacles in the environment allow sound to pass through them as if they (the obstacles) were not there</li>
 * </ul>
 * AudioHandler is accessed by the EPuck class.
 *
 * The AudioHandler code is not intended to be user facing, the user interacts with it using the EPuck API.
 *
 * AudioHandler uses the Singleton design pattern, so there can only ever be one instance of it. Constructor is called using the Instance() method.
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#ifndef AUDIOHANDLER_H_
#define AUDIOHANDLER_H_

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include "libplayerc++/playerc++.h"

#define SAMPLE_RATE					16000
#define FFT_BLOCK_SIZE				128
#define PI							3.141592654
#define IMPEDANCE_OF_SPEAKER_OHMS 	10



//using namespace PlayerCc;

//TODO make getTones sum levels of all tones in the area and return that as sound level
//TODO make getTones use x and y of each tone instead of the apparent tone bullshizzle.

/**
 * takes care of all audio processing and collection, stores audio data and takes care of returning it to the calling object.
 * Audio in the simulation is global so AudioHandler is handled as a singleton and there can be only one AudioHandler,
 * which contains all the audio data in the environment.
 **/
class AudioHandler
{
public:

	/**
	 * This data structure is used to send audio data from the AudioHandler to the EPuck API
	 * */
	typedef struct audio_message
	{
		/**the frequency of the tone heard*/
		double frequency;
		/**The direction, with respect to the robot's current heading, that the sound came from.
		 * In degrees, in front of the robot being 0 and anti-clockwise from there increasing.*/
		int direction;
		/**Some volume level given to the tone. Can indicate how far away the tone is possibly.*/
		double volume;

	}audio_message_t;




	/** AudioBin stores information about a tone in the environment. Each object represents a frequency band containing tones being played.
	 * */
	class AudioBin
	{
	public:
		/**Represents a tone being played*/
		typedef struct audio_tone_t
		{
			/**The x coord of tone source*/
			double tx;
			/**The y coord of tone source*/
			double ty;
			/**The sound intensity level of the tone at source. Watts are used instead of volts (which is what the user gives) because this
			 * takes into account the impedance of the speaker and the microphones.*/
			double wattsAtSource;
			/**Time that the tone will stop playing*/
			double end;
			/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
			audio_tone_t *next;
			audio_tone_t *previous;
		}audio_tone_t;

		/**frequency bin lower bound*/
		double lowerFrequencyBound;
		/**Linked list of tones currently in this bin*/
		audio_tone_t *tones;

		/**AudioBins are stored as a linked list by the handler so need to be able to navigate LL.*/
		AudioBin *next;
		AudioBin *previous;

		/**
		 * Creates a new AudioBin item. The AudioBin linked list uses the lower frequency bound as a key, so this must be given
		 * upon initialisation.
		 * @param lowerfreq the lower frequency bound of this audio bin.
		 * @param prev the previous AudioBin in the Linked list the bins are stored in.
		 * @param nxt the next AudioBin in the Linked list the bins are stored in.
		 * */
		AudioBin(double lowerFreq, AudioBin *prev, AudioBin *nxt);

		virtual ~AudioBin();

		/**Updates list of tones in the bin so that ones which have finished playing are removed.
		 * Also updates apparent sound sources
		 * @param currentTime the current time of the simulation
		 * @returns status 0 if a tone is removed but there are still tones in the AudioBin, 1 if entire list is now empty.*/
		int updateList(double currentTime);

		/**
		 * Adds an audio_tone_t into the AudioBin. Creates an audio_tone_t object, appends it to the linked list and updates itself.
		 * @param x the x position of the robot playing the tone
		 * @param y the y position of the robot playing the tone
		 * @@param volume how loud to play the tone at. Value indicates in VOLTS how big the signal to the speaker should be.
		 * This must be in the range 0 to 5, if this number is outside the range it will be assumed to be either 0 or 5 (whichever is closer).
		 * So if you set this as 3, the speaker will play a sine wave at 3 volts (peak to peak).
		 * @param endtime the simulated time at which the tone will end.
		 * */
		void addTone(double x, double y, double voltage, double endtime);

		/**
		 * Given the position and yaw of the robot this function calculates the cumulative level and direction of the tones in this frequency bin.
		 * This is written to an audio_message_t data structure, the pointer to which the calling function mus supply.
		 * @param x the x position of the robot
		 * @param y the y position of the robot
		 * @param yaw the yaw of the robot. In radians because that's the measure used by playerstage
		 * @param output the audio_message_t pointer where the data should be stored.
		 * */
		int calculateCumulativeDataForPosition(double x, double y, double yaw, audio_message_t* output);

	private:
		/**
		 * Removes the supplied audioTone from the linked list.
		 * @param *del address of the tone to delete.
		 * @returns success 0 if tone is deleted, 1 if tone is deleted and the bin is now empty.
		 * */
		int removeTone(audio_tone_t *del);

		/**
		 * Function to convert a distance in metres into a volume for the robots.
		 * @param levelAtSource the number of watts produced at by the speaker at the tone source.
		 * @param distance the distance in metres
		 * @returns intensity the sound intensity of the tone in W/m^2. This is then used in {@link AudioBin#addTwoSoundIntensities }
		 * */
		double getSoundIntensity(double levelAtSource, double distance);

		/**
		 * Adds one sound intensity level to another and gives the resulting sound intensity level.
		 * All measures are assumed to be in W/m^2. I am not an acoustician this means nothing to me.
		 * Numbers go in, number come out, what more do I need to know? I found the formula at: <br>
		 * http://www.suite101.com/content/how-loud-is-it-a62825
		 * @param sound1 level of a sound in W/m^2.
		 * @param sound2 level of a sound in W/m^2.
		 * @returns soundSum combined level of the two sounds in dB SPL.
		 * */
		double addTwoSoundIntensities(double sound1, double sound2);

		/**
		 * Function to convert two coordinates into a bearing.
		 * Takes the coordinates of the sound source and the sound reciever, and works out the bearing of the sound source wrt the reciever.
		 * Bearings are measured anticlockwise from where the robot is facing straight ahead, the unit used is degrees (not radians).
		 * @param xdiff the source x coordinate minus the reciever x coordinate
		 * @param ydiff the source y coordinate minus the reciever y coordinate
		 * @param recieverYaw the yaw of the robot that is listening for tones.
		 * */
		int convertDifferentialCoordsIntoBearing(double xdiff, double ydiff, double recieverYaw);

		//==== USEFUL STUFF ====

		/**
		 * converts radians to degrees.
		 * */
		int radiansToDegrees(double rads);

		/**
		 * converts degrees to radians
		 * */
		double degreesToRadians(int degrees);

		/**
		 * Rounds the given number to the nearest multiple of the "resolution" parameter.
		 * e.g. if input is 23.720954 and resolution is 5 this will return 25.
		 * e.g. if input is 23.720954 and resolution is 0.5 this will return 23.5.
		 * @param input the number to round
		 * @param resolution the resolution to round to
		 * @returns out the answer.
		 * */
		double roundToNearest(double input, double resolution);
	};








	//member variables
	//linked list of AudioBins for the frequencies currently being played.
	AudioBin *environment;
	double lowerFFTBounds[FFT_BLOCK_SIZE/2];
	int numberOfBins;

	//player stuff
	PlayerCc::SimulationProxy	*simProxy;
	PlayerCc::PlayerClient *simClient;
	char aRobotName[32];

	//function prototypes

	//singleton thing...
	/**
	 * Function that allows AudioHandler to be a singleton. Use this to construct the AudioHandler object.
	 * @param simulationClient the PlayerClient object which handles the simulation.
	 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
	 * @param name the name of the robot that initialises the AudioHandler.
	 * This is used for accessing data from the simulation proxy, as you need the name of a model to get simulation time information.
	 * */
	static AudioHandler* GetAudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name);

	virtual ~AudioHandler();

	//methods.
	/**
	 * Puts a tone of the desired frequency and duration into the audio environment.
	 * @param freq the frequency of the tone in Hz
	 * @param duration the length of the tone in milliseconds
	 * @param volume how loud to play the tone at. Value indicates in VOLTS how big the signal to the speaker should be.
	 * This must be in the range 0 to 5, if this number is outside the range it will be assumed to be either 0 or 5 (whichever is closer).
	 * So if you set this as 3, the speaker will play a sine wave at 3 volts (peak to peak).
	 * @param name string containing the name of the robot playing the tone.
	 * */
	void playTone(int freq, double duration, double voltage, char* name);

	/**
	 * Returns the number of AudioBins currently in the environment. This function is needed so that space can be allocated for the Tones in the EPuck code.
	 * @returns notones the number of different frequency tones the robot can detect.
	 * */
	int getNumberOfTones(void);

	/**
	 * Provides the audio data in the environment, including frequency, volume and direction of the tones.
	 * The function which calls this must allocate the memory needed as an array, and provide the address and size of the memory slot.
	 * This function will then copy the environmental audio data into the provided memory using the audio_message_t structure.
	 * @param robotName	the name of the robot which is requesting the data (this is given in the worldfile)
	 * @param store		link to where the audio data memory has been allocated.
	 * @param storesize	the number of audio_message_t objects allocated to the audio data.
	 * @returns success 0 if successfully copied data, 1 if unsuccessful (like say if new data has been added between allocating memory and trying to copy it over).
	 * @see AudioHandler#getNumberOfTones()
	 * @see
	 * Example code:<br>
	 * {@code audio_message_t *noise = new audio_message_t[AudioHandler.getNumberOfTones()];}
	 * {@code getTones(robotname, noise, sizeof(audio_message_t)*AudioHandler.getNumberOfTones());}
	 * */
	int getTones(char* robotName, audio_message_t *store, size_t storesize);


	/** Test function to print all of the sound environment data to stdout.
	 * */
	void dumpData_TEST(void);

protected:
	//protected so it can be a singleton
	/**
	 * Creates the audiohandler object and builds an array to store the sound data for each robot.
	 * @param simulationClient the PlayerClient object which handles the simulation.
	 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
	 * @param name the name of the robot that initialises the AudioHandler.
	 * This is used for accessing data from the simulation proxy, as you need the name of a model to get simulation time information.
	 * */
	AudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name);

private:
	//singleton reference to only instance of audiohandler.
	static AudioHandler* _instance;

	pthread_t updateAudioBinListThread;


	/**
	 * Removes an AudioBin from the Linked list.
	 * @param del AudioBin to remove from LL.
	 * */
	int removeBin(AudioBin *del);


	/**
	 * Updates the list of AudioBins so that tones which have finished playing are removed from data storage.
	 * This function is intended to be threaded, so contains a loop which does not terminate.
	 * */
	void updateAudioBinListThreaded(void);

	/**
	 * Fancy function for kickstarting the thread that checks the AudioBins.
	 * */
	static void *startupdateAudioBinListThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<AudioHandler *>(obj)->updateAudioBinListThreaded();
		return NULL;
	}
};



#endif /* AUDIOHANDLER_H_ */
