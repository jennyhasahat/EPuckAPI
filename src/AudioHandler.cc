/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "AudioHandler.h"




//I hope this statement doesn't have global scope... I don't think it does.
AudioHandler* AudioHandler::_instance = 0;


AudioHandler* AudioHandler::GetAudioHandler(PlayerClient *simulationClient, SimulationProxy *sim)
{
	if(_instance == 0)
	{
		_instance = new AudioHandler(simulationClient, sim);
	}
	return _instance;
}


AudioHandler::AudioHandler(PlayerClient *simulationClient, SimulationProxy *sim)
{
	int i;

	simClient = simulationClient;
	simProxy = sim;
	environment = NULL;

	//make array of frequency bins depending on FFT settings.
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		lowerFFTBounds[i] = (i*SAMPLE_RATE)/FFT_BLOCK_SIZE;
	}

	testInitialisation_TEST();
	printf("AudioHandler initialised\n");
	return;
}

AudioHandler::~AudioHandler()
{
	delete[] environment;

	return;
}


void AudioHandler::playTone(int freq, double duration, char* name)
{
	int whichbin;
	//AudioBin current = environment;

	//find FFT lower frequency bound for freq
	for(whichbin=(FFT_BLOCK_SIZE/2)-1; whichbin>-1; whichbin--)
	{
		if(lowerFFTBounds[whichbin] <= freq) break;
	}

	//check linked list to see if there is an audio bin for this sound
	//if there is no audio bin for the sound then make one

	//add data to the audio bin entry




	//if so, append this tone to the end of the tone list and update the list.
	//if not, create a new audio bin and populate it with data from the epuck


	return;
}



int AudioHandler::testInitialisation_TEST(void)
{
	int i;

	printf("The FFT lower bounds are:\n");
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		printf("%f ", lowerFFTBounds[i]);
	}
	printf("\n");

	dumpData_TEST();

	return 0;
}


//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================


void AudioHandler::dumpData_TEST(void)
{
	AudioBin *binptr = environment;
	AudioBin::AudioTone *toneptr;

	printf("AudioHandler stored data:\n");

	while(binptr != NULL)
	{
		printf("Bin lower bound is %f\n", binptr->lowerFrequencyBound);
		printf("\tapparent x is %f\n", binptr->x);
		printf("\tapparent y is %f\n", binptr->y);
		printf("Stored tones:\n");
		toneptr = binptr->tones;
		while(toneptr != NULL)
		{
			printf("\t\tx: %f, y: %f, end: %f\n", toneptr->x, toneptr->y, toneptr->end);
			toneptr = toneptr->next;
		}
		binptr = binptr->next;
	}

	return;
}






