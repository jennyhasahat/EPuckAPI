/**
 * AudioHandler stores the audio environment data in the simulation.
 * It provides interfaces for storing audio information and for accessing audio information.
 * A number of simplifications about sound have been made in this class for the purpose of creating a simulation:<p>
 * <ul>
 * <li>Sounds a combined linearly</li>
 * <li>there is no interference between tones of different frequencies</li>
 * <li>Sound decays linearly over distance at a rate of initial_level / distance_away_in_m </li>
 * <li>All frequencies decay over distance at the same rate</li>
 * <li>The environment has no reverberations to confuse the direction and level of a sound</li>
 * <li>Robots don't move whilst making a sound</li>
 * <li>All robots play a tone at the same volume</li>
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
#include <pthread.h>
#include "libplayerc++/playerc++.h"

#define SAMPLE_RATE		16000
#define FFT_BLOCK_SIZE	128

//using namespace PlayerCc;



class AudioHandler
{
public:

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
			/**Time that the tone will stop playing*/
			double end;
			/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
			audio_tone_t *next;
			audio_tone_t *previous;
		}audio_tone_t;

		/**frequency bin lower bound*/
		double lowerFrequencyBound;
		/**apparent x coordinate of tone, calculated by taking the mean of all the x values of currently playing audio_tone_t objects.*/
		double x;
		/**apparent y coordinate of tone, calculated by taking the mean of all the y values of currently playing audio_tone_t objects.*/
		double y;
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
		 * @param endtime the simulated time at which the tone will end.
		 * */
		void addTone(double x, double y, double endtime);

	private:
		/**
		 * Removes the supplied audioTone from the linked list.
		 * @param *del address of the tone to delete.
		 * @returns success 0 if tone is deleted, 1 if tone is deleted and the bin is now empty.
		 * */
		int removeTone(audio_tone_t *del);

		/**
		 * Updates the apparent position of the audio tone.
		 * This method should be called whenever a new tone is added to the list, or whenever a tone is removed.
		 * */
		void updatePosition(void);

	};


	/**
	 * This data structure is used to send audio data from the AudioHandler to the EPuck API
	 * */
	typedef struct audio_message
	{
		/**the frequency of the tone heard*/
		double frequency;
		/**The direction, with respect to the robot's current heading, that the sound came from.
		 * In degrees, in front of the robot being 0 and anti-clockwise from there increasing.*/
		double direction;
		/**Some volume level given to the tone. Can indicate how far away the tone is possibly.*/
		double volume;

	}audio_message_t;






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
	 * @param name string containing the name of the robot playing the tone.
	 * */
	void playTone(int freq, double duration, char* name);

	/**
	 * Returns the number of AudioBins currently in the environment. This function is needed so that space can be allocated for the Tones in the EPuck code.
	 * @returns notones the number of different frequency tones the robot can detect.
	 * */
	int numberOfTones(void);

	/**
	 * Provides the audio data in the environment, including frequency, volume and direction of the tones.
	 * The function which calls this must allocate the memory needed as an array, and provide the address and size of the memory slot.
	 * This function will then copy the environmental audio data into the provided memory using the audio_message_t structure.
	 * @param robotName	the name of the robot which is requesting the data (this is given in the worldfile)
	 * @param store		link to where the audio data memory has been allocated.
	 * @param storesize	the number of bytes allocated to the audio data.
	 * @returns success 0 if successfully copied data, 1 if unsuccessful (like say if new data has been added between allocating memory and trying to copy it over).
	 * @see AudioHandler#numberOfTones()
	 * Example code:<br>
	 * {@code audio_message_t *noise = new audio_message_t[AudioHandler.numberOfTones()];}
	 * {@code getTones(robotname, noise, sizeof(audio_message_t)*AudioHandler.numberOfTones());}
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
	 * Function to convert a distance in metres into a volume for the robots.
	 * @param distance the distance in metres
	 * @returns volume the volume of the tone at that distance, normalised between 0 and 1
	 * */
	double convertDistanceIntoSoundLevel(double distance);

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
