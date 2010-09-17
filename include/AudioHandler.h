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

	/**Represents a tone being played*/
	class AudioTone
	{
	public:
		/**The x coord of tone source*/
		double x;
		/**The y coord of tone source*/
		double y;
		/**Time that the tone will stop playing*/
		clock_t end;
		/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
		AudioTone *previous;
		AudioTone *next;
	};

	/** AudioData stores information about a tone in the environment. Each object represents a frequency band containing tones being played.
	 * */
	class AudioBin
	{
	public:
		/**frequency bin lower bound*/
		double lowerFrequencyBound;
		/**apparent x coordinate of tone*/
		double x;
		/**apparent y coordinate of tone*/
		double y;
		/**Linked list of tones currently in this bin*/
		AudioTone *tones;

		/**Updates list of tones in the bin so that ones which have finished playing are removed.
		 * Also updates apparent sound sources
		 * @param currentTime the current time of the simulation
		 * @returns status 1 if entire list is now empty, 0 if a tone is removed but there are still tones in the AudioBin.*/
		int updateList(clock_t currentTime);
	};


	//member variables
	AudioBin *environment;
	double lowerFFTBounds[FFT_BLOCK_SIZE/2];

	//player stuff
	SimulationProxy	*simProxy;
	PlayerClient *simClient;

	//function prototypes

	//singleton thing...
	static AudioHandler* GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim);

	virtual ~AudioHandler();

	//methods.
	void playTone(int freq, double duration);
	int testInitialisation(void);

protected:
	//protected so it can be a singleton
	AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim);

private:
	//singleton reference to only instance of audiohandler.
	static AudioHandler* _instance;

	int findRobotSlot(char *name);
	void dumpData(void);


};



#endif /* AUDIOHANDLER_H_ */
