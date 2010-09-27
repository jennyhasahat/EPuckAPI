/**
 * AudioHandler stores the audio environment data in the simulation.
 * It provides interfaces for storing audio information and for accessing audio information.
 * AudioHandler is not meant to be user-facing and should be accessed through the EPuck class.
 *
 * The AudioHandler code is not intended to be user facing, the user interacts with it through the EPuck API.
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
#include "libplayerc++/playerc++.h"

#define SAMPLE_RATE		16000
#define FFT_BLOCK_SIZE	128

using namespace PlayerCc;



class AudioHandler
{
public:

	/** AudioBin stores information about a tone in the environment. Each object represents a frequency band containing tones being played.
	 * */
	class AudioBin
	{
	public:

		/**Represents a tone being played*/
		typedef struct AudioTone
		{
			/**The x coord of tone source*/
			double x;
			/**The y coord of tone source*/
			double y;
			/**Time that the tone will stop playing*/
			clock_t end;
			/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
			AudioTone *next;
			AudioTone *previous;
		}AudioTone;

		/**frequency bin lower bound*/
		double lowerFrequencyBound;
		/**apparent x coordinate of tone, calculated by taking the mean of all the x values of currently playing AudioTone objects.*/
		double x;
		/**apparent y coordinate of tone, calculated by taking the mean of all the y values of currently playing AudioTone objects.*/
		double y;
		/**Linked list of tones currently in this bin*/
		AudioTone *tones;

		/**AudioBins are stored as a linked list by the handler so need to be able to navigate LL.*/
		AudioBin *next;
		AudioBin *previous;

		/**
		 * Creates a new AudioBin item. The AudioBin linked list uses the lower frequency bound as a key, so this must be given
		 * upon initialisation.
		 * @param lowerfreq the lower frequency bound of this audio bin.
		 * */
		AudioBin(double lowerFreq);

		virtual ~AudioBin();

		/**Updates list of tones in the bin so that ones which have finished playing are removed.
		 * Also updates apparent sound sources
		 * @param currentTime the current time of the simulation
		 * @returns status 0 if a tone is removed but there are still tones in the AudioBin, 1 if entire list is now empty.*/
		int updateList(clock_t currentTime);

		/**
		 * Adds an AudioTone into the AudioBin. Creates an AudioTone object, appends it to the linked list and updates itself.
		 * @param x the x position of the robot playing the tone
		 * @param y the y position of the robot playing the tone
		 * @param endtime the system time at which the tone will end.
		 * */
		void addTone(double x, double y, clock_t endtime);

	private:
		/**
		 * Removes the supplied audioTone from the linked list.
		 * @param *del address of the tone to delete.
		 * @returns success 0 if tone is deleted, 1 if tone is deleted and the bin is now empty.
		 * */
		int removeTone(AudioTone *del);

		/**
		 * Updates the apparent position of the audio tone.
		 * This method should be called whenever a new tone is added to the list, or whenever a tone is removed.
		 * */
		void updatePosition(void);
	};










	//member variables
	//linked list of AudioBins for the frequencies currently being played.
	AudioBin *environment;
	double lowerFFTBounds[FFT_BLOCK_SIZE/2];

	//player stuff
	SimulationProxy	*simProxy;
	PlayerClient *simClient;

	//function prototypes

	//singleton thing...
	/**
	 * Function that allows AudioHandler to be a singleton. Use this to construct the AudioHandler object.
	 * @param simulationClient the PlayerClient object which handles the simulation.
	 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
	 * */
	static AudioHandler* GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim);

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
	 * test function to make sure the audio handler is initialised.
	 * */
	int testInitialisation_TEST(void);

protected:
	//protected so it can be a singleton
	/**
	 * Creates the audiohandler object and builds an array to store the sound data for each robot.
	 * @param simulationClient the PlayerClient object which handles the simulation.
	 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
	 * */
	AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim);

private:
	//singleton reference to only instance of audiohandler.
	static AudioHandler* _instance;

	/** Private test function to print all of the sound environment data to stdout.
	 * */
	void dumpData_TEST(void);


};



#endif /* AUDIOHANDLER_H_ */
