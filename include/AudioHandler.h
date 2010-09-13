/**
 * AudioHandler stores the audio environment data in the simulation.
 * It provides interfaces for storing audio information and for accessing audio information.
 * AudioHandler is not meant to be user-facing and should be accessed through the EPuck class. However, because it contains
 *
 * The AudioHandler code is not intended to be user facing, the user interacts with it through the EPuck API.
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#ifndef AUDIOHANDLER_H_
#define AUDIOHANDLER_H_

#include <stdio.h>
#include <time.h>
#include "libplayerc++/playerc++.h"

using namespace PlayerCc;



class AudioHandler
{
public:

	/** AudioData stores information about a tone in the environment. Each object represents a tone being played.
	 * */
	class AudioData
	{
	public:
		/**frequency of the tone*/
		int frequency;
		/**x coordinate of robot playing tone*/
		double x;
		/**y coordinate of robot playing tone*/
		double y;
		/**yaw of robot playing tone*/
		double yaw;
		/**time that the tone started playing*/
		clock_t start;
		/**Length of the tone in milliseconds*/
		double duration;
		/**Time that the tone will stop playing*/
		clock_t end;
	};

	//member variables
	char** robotNames;
	int numberRobots; //number of robots in whole simulation
	AudioData *environment;
	//bool initialised;


	//player stuff
	SimulationProxy	*simProxy;

	//function prototypes

	AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots);
	virtual ~AudioHandler();

	void playTone(int id, int freq, double duration);
	int initialiseEPuck(char* robotname);

private:
	int findRobotSlot(char *name);
	void dumpData(void);


};



#endif /* AUDIOHANDLER_H_ */
