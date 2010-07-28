/**
 * AudioHandler is an interface for the stageaudio plugin.
 * It stores the data recorded by the stageaudio driver and interprets that into a set of frequencies and the
 * direction relative to the robot that they come from. AudioHandler also deals with sending data to the driver.
 *
 * The AudioHandler code is not intended to be user facing, the user interacts with it through the EPuck API.
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#ifndef AUDIOHANDLER_H_
#define AUDIOHANDLER_H_

#include <stdio.h>
#include "libplayerc++/playerc++.h"

using namespace PlayerCc;

class AudioHandler
{
public:
	//member variables
	int numberRobots;

	//player stuff
	SimulationProxy	*simProxy;
	BlackBoardProxy	*blackBProxy;
	player_blackboard_entry_t *allsounds;

	//function prototypes

	AudioHandler(BlackBoardProxy *bbp, SimulationProxy *sim, int nobots);
	virtual ~AudioHandler();

};

#endif /* AUDIOHANDLER_H_ */
