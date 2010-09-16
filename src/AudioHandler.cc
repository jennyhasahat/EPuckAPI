/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "AudioHandler.h"


//I hope this statement doesn't have global scope... I don't think it does.
AudioHandler* AudioHandler::_instance = 0;

/**
 * Function that allows AudioHandler to be a singleton. Use this to construct the AudioHandler object.
 * @param simulationClient the PlayerClient object which handles the simulation.
 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
 * @param nobots the number of robots in the simulation.
 * */
AudioHandler* AudioHandler::GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots)
{
	if(_instance == 0)
	{
		_instance = new AudioHandler(simulationClient, sim, nobots);
	}
	return _instance;
}

/**
 * Creates the audiohandler object and builds an array to store the sound data for each robot.
 * @param simulationClient the PlayerClient object which handles the simulation.
 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
 * @param nobots the number of robots in the simulation.
 * */
AudioHandler::AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim, int nobots)
{
	int i;

	simProxy = sim;
	numberRobots = nobots;

	environment = new AudioData[numberRobots];

	//create an array of strings for the robot names
	robotNames = new char*[numberRobots];

	//robotNames = (char**)calloc(numberRobots, sizeof(char*));
	for(i=0;i<numberRobots;i++)
	{
		robotNames[i] = new char[32];
		//robotNames[i] = (char*)calloc(maxKeyNameLength, sizeof(char));
	}



	printf("Audio handler initialised?\n");
	return;
}

AudioHandler::~AudioHandler()
{
	int i;

	delete[] environment;

	for(i=0;i<numberRobots;i++)
	{
		delete[] robotNames[i];
	}
	delete[] robotNames;

	return;
}

/**
 * Searches list of robots and indexes to find an empty index.
 * If the robot is already assigned an index it returns this instead.
 * @param name name of the robot that it is trying to find a slot for.
 * @returns index the index to store that robot's data in.
 * */
int AudioHandler::initialiseEPuck(char *name)
{
	int i;

	//scroll through ALL keys until you find an entry matching the provided key name
	// if none is found then give the first empty key in our list.
	for(i=0;i<numberRobots;i++)
	{
		//	printf("%d) about to compare \"%s\" and \"%s\"\n", i, recieved.key, allkeys[i]);
		//if the given key and the current one match then stop searching
		if(!strncmp(name, robotNames[i], strlen(name)))
		{
			return i;
		}

		//if key we're looking at is empty then we've reached the end of the list of filled slots and should allocate this slot
		if(strlen(robotNames[i]) == 0)
		{
			//fill slot with this robot
			strncpy(robotNames[i], name, 32);
			return i;
		}
	}

	return -1;
}

/**
 * Puts a tone of the desired frequency and duration into the audio environment.
 * @param id the index of the robot which wants to put info in the environment
 * @param freq the frequency of the tone in Hz
 * @param duration the length of the tone in milliseconds
 * */
void AudioHandler::playTone(int id, int freq, double duration)
{
	char name[32];

	environment[id].frequency = freq;
	//get robot's name
	strcpy(name, robotNames[id]);
	//get robot's position
	simProxy->GetPose2d(name, environment[id].x, environment[id].y, environment[id].yaw);

	//save timestamp clock() is more accurate than time(NULL).
	environment[id].start = (clock()*1000)/CLOCKS_PER_SEC;
	environment[id].duration = duration;
	environment[id].end = environment[id].start + (CLOCKS_PER_SEC*(duration/1000));

	printf("tone start: %f, end: %f\n", environment[id].start, environment[id].end);
	return;
}


/**
 * test function to make sure the audio handler is initialised.
 * */
int AudioHandler::testInitialisation(void)
{
	int i;

	printf("The FFT lower bounds are:\n");
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		printf("%f ", FFTLowerBounds[i]);
	}
	return numberRobots;
}


//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================

/** Private test function to print all of the sound environment data to stdout.
 * */
void AudioHandler::dumpData(void)
{
	int i;

	for(i=0;i<numberRobots;i++)
	{
		printf("Robot number %d, named %s\n", i, robotNames[i]);
		printf("\tfrequency %d Hz\n", environment[i].frequency);
		printf("\tx: %f, y: %f, yaw: %f\n", environment[i].x, environment[i].y, environment[i].yaw);
		printf("\tstart time: %f\n\tend time: %f\n", environment[i].start, environment[i].end);
		printf("\tduration: %f msecs.\n", environment[i].duration);
	}
	return;
}




/**
 * Searches list of robots and indexes to find an empty index.
 * If the robot is already assigned an index it returns this instead.
 * @param name name of the robot that it is trying to find a slot for.
 * @returns index the index to store that robot's data in.
 * */
/*
int AudioHandler::findRobotSlot(char *name)
{
	int i;

	//scroll through ALL keys until you find an entry matching the provided key name
	// if none is found then give the first empty key in our list.
	for(i=0;i<numberRobots;i++)
	{
		//	printf("%d) about to compare \"%s\" and \"%s\"\n", i, recieved.key, allkeys[i]);
		//if the given key and the current one match then stop searching
		if(!strncmp(name, robotNames[i], strlen(name)))
		{
			return i;
		}

		//if key we're looking at is empty then we've reached the end of the list of filled slots and should allocate this slot
		if(strlen(robotNames[i]) == 0)
		{
			//fill slot with this robot
			strncpy(robotNames[i], name, 32);
			return i;
		}
	}

	return -1;
}
*/


