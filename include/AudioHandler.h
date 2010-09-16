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

	/** AudioData stores information about a tone in the environment. Each object represents a tone being played.
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
		/**time that the tone started playing*/
		clock_t start;
		/**Length of the tone in milliseconds*/
		double duration;
		/**Time that the tone will stop playing*/
		clock_t end;
	};

	//member variables
	int numberRobots; //number of robots in whole simulation
	AudioBin *environment;

	//player stuff
	SimulationProxy	*simProxy;
	PlayerClient *simClient;

	//function prototypes

	//singleton thing...
	static AudioHandler* GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots);

	virtual ~AudioHandler();

	//methods.
	void playTone(int id, int freq, double duration);
	int initialiseEPuck(char* robotname);
	int testInitialisation(void);

protected:
	//protected so it can be a singleton
	AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots);

private:
	//singleton reference to only instance of audiohandler.
	static AudioHandler* _instance;

	int findRobotSlot(char *name);
	void dumpData(void);


};



#endif /* AUDIOHANDLER_H_ */
