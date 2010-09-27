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
	//remove all bins in Linked list
	AudioBin *ptr = environment;

	while(ptr->next != NULL)
	{
		ptr = ptr->next;
		delete ptr->previous;
	}
	delete ptr;

	return;
}


void AudioHandler::playTone(int freq, double duration, char* name)
{
	int whichbin;
	double x, y, yaw;
	AudioBin *current = environment;
	AudioBin *last;

	//find FFT lower frequency bound for freq
	for(whichbin=(FFT_BLOCK_SIZE/2)-1; whichbin>-1; whichbin--)
	{
		if(lowerFFTBounds[whichbin] <= freq) break;
	}

	//check linked list to see if there is an audio bin for this sound
	while(current != NULL)
	{
		if(current->lowerFrequencyBound == lowerFFTBounds[whichbin]) break;
		last = current;
		current = current->next;
	}

	//if there is no audio bin for the sound then make one
	if(current == NULL)
	{
		current = new AudioBin(lowerFFTBounds[whichbin], last, NULL);
	}

	//add data to the audio bin entry
	simProxy->GetPose2d(name, x, y, yaw);
	printf("robot is at x: %f, y: %f yaw: %f\n", x, y, yaw);

	//todo fix the next line when clock stuff is sorted.
	current->addTone(x, y, 500);



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
	printf("pointer is at %x\n", binptr);
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



int AudioHandler::removeBin(AudioBin *del)
{
	//Four possibilities for this, each needing different stuff doing.
	//case 1, tone is the first in the list.
		//case 1a tone is the only thing in the list
	//case 2, tone is in the middle
	//case 3 tone is at the end of the list

	//if next is not null then update its previous pointer to this.previous
	if(del->next != NULL) del->next->previous = del->previous;

	//if previous is not null update its next pointer to this.next
	if(del->previous != NULL) del->previous->next = del->next;

	//is this the first tone?
	if(del->previous == NULL)
	{
		//update tones to point to first item in list
		environment = del->next;

		//is this the only tone?
		if(del->next == NULL)
		{
			//delete tone and flag as empty.
			delete del;
			return 1;
		}
	}

	delete del;
	return 0;
}




