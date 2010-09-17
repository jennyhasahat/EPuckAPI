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
		lowerFFTBounds[i] = (i*SAMPLE_RATE)/FFT_BLOCK_SIZE;
		//environment[i].lowerFrequencyBound = (i*SAMPLE_RATE)/FFT_BLOCK_SIZE;
	}

	printf("Audio handler initialised?\n");
	testInitialisation();
	return;
}

AudioHandler::~AudioHandler()
{
	delete[] environment;

	return;
}

/**
 * Puts a tone of the desired frequency and duration into the audio environment.
 * @param freq the frequency of the tone in Hz
 * @param duration the length of the tone in milliseconds
 * */
void AudioHandler::playTone(int freq, double duration)
{
	//find which bin to put the sound into


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

	dumpData();

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
		//printf("\tstart time %f, end time %f\n", environment[i].start, environment[i].end);
	//	printf("\tduration: %f\n", environment[i].duration);
	}


	return;
}







