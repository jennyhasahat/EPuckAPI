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
 * */
AudioHandler* AudioHandler::GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim)
{
	if(_instance == 0)
	{
		_instance = new AudioHandler(simulationClient, sim);
	}
	return _instance;
}

/**
 * Creates the audiohandler object and builds an array to store the sound data for each robot.
 * @param simulationClient the PlayerClient object which handles the simulation.
 * @param simProxy the simulationProxy attached to the playerclient handling this simulation.
 * */
AudioHandler::AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim)
{
	int i;

	simClient = simulationClient;
	simProxy = sim;

	environment = new AudioBin[FFT_BLOCK_SIZE/2];

	//make array of frequency bins depending on FFT settings.
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		environment[i].lowerFrequencyBound = (i*SAMPLE_RATE)/FFT_BLOCK_SIZE;
	}

	printf("Audio handler initialised?\n");
	return;
}

AudioHandler::~AudioHandler()
{
	delete[] environment;

	return;
}

/**
 * Puts a tone of the desired frequency and duration into the audio environment.
 * @param id the index of the robot which wants to put info in the environment
 * @param freq the frequency of the tone in Hz
 * @param duration the length of the tone in milliseconds
 * */
void AudioHandler::playTone(int freq, double duration)
{
	testInitialisation();
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
		printf("%f ", environment[i].lowerFrequencyBound);
	}
	printf("\nthere are %d robots\n", numberRobots);

	return 0;
}


//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================

/** Private test function to print all of the sound environment data to stdout.
 * */
void AudioHandler::dumpData(void)
{
	int i;

	printf("AudioHandler stored data:\n");
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		printf("frequency bin %f\n", environment[i].lowerFrequencyBound);
		printf("\tapparent pose x: %f y: %f\n", environment[i].x, environment[i].y);
		printf("\tstart time %f, end time %f\n", environment[i].start, environment[i].end);
		printf("\tduration: %f\n", environment[i].duration);
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


