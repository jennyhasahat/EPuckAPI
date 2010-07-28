/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "AudioHandler.h"

AudioHandler::AudioHandler(BlackBoardProxy *bbp, SimulationProxy *sim, int nobots, char *name)
{
	//initialise member variables
	blackBProxy = bbp;
	simProxy = sim;
	numberRobots = nobots;
	strncpy()

	//create array for sound data
	allsounds = new player_blackboard_entry_t[numberRobots];

}

AudioHandler::~AudioHandler()
{
	delete[] allsounds;
}
