/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "AudioHandler.h"

AudioHandler::AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots, char *name)
//AudioHandler::AudioHandler(BlackBoardProxy *bbp, SimulationProxy *sim, int nobots, char *name)
{
	// group name to send driver. Needs to be parameter nobots in a string format
	char group[8];
	// data returned from driver
	player_blackboard_entry_t packet, *p2;

	//initialise member variables
	blackBProxy = new BlackBoardProxy(simulationClient, 0);
	//blackBProxy = bbp;
	simProxy = sim;
	numberRobots = nobots;
	strncpy(robotName, name, 32);

	sprintf(group, "%d", nobots);
	packet.key = robotName;
	packet.group = group;
	packet.type = 0;

	printf("handler sending %s and %s to blackboard driver\n", robotName, group);
	p2 = blackBProxy->GetEntry(robotName, group);
	printf("handler recieved data packet back\n");

	allsounds = (player_blackboard_entry_t**)p2->group;
	mysounds = (player_blackboard_entry_t*)p2->data;

	printf("driver sent data all %x and this %x\n", allsounds, mysounds);
}

AudioHandler::~AudioHandler()
{
	delete[] allsounds;
}


void AudioHandler::playTone(int freq, double duration)
{
	double x, y, dummy;

	simProxy->GetPose2d(robotName, x, y, dummy);

	//create packet of data to send
	player_blackboard_entry_t packet;

	//populate packet with data
	/* Key is robot name
	 * data count is frequency
	 * timestamp sec is time that the request was sent
	 * timestamp usec is duration in milliseconds
	 */
	packet.key = robotName;
	packet.key_count = strlen(robotName);
	packet.data_count = freq;


}
