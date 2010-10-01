/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "AudioHandler.h"




//I hope this statement doesn't have global scope... I don't think it does.
AudioHandler* AudioHandler::_instance = 0;


AudioHandler* AudioHandler::GetAudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name)
{
	if(_instance == 0)
	{
		_instance = new AudioHandler(simulationClient, sim, name);
	}
	return _instance;
}


AudioHandler::AudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name)
{
	int i;

	simClient = simulationClient;
	simProxy = sim;
	environment = NULL;
	strncpy(aRobotName, name, 32);
	numberOfBins = 0;

	//make array of frequency bins depending on FFT settings.
	for(i=0; i<FFT_BLOCK_SIZE/2; i++)
	{
		lowerFFTBounds[i] = (i*SAMPLE_RATE)/FFT_BLOCK_SIZE;
	}

	pthread_create(&updateAudioBinListThread, 0, AudioHandler::startupdateAudioBinListThread, this);

	printf("AudioHandler initialised\n");
	return;
}

AudioHandler::~AudioHandler()
{
	//close thread
	pthread_detach(updateAudioBinListThread);

	//remove all bins in Linked list
	AudioBin *ptr = environment;
	AudioBin *prev;

	while(ptr != NULL)
	{
		prev = ptr;
		ptr = ptr->next;
		delete prev;
	}

	printf("AudioHandler destroyed.\n");
	return;
}


void AudioHandler::playTone(int freq, double duration, char* robotName)
{
	int whichbin;
	char timeflag[] = "sim_time";

	double x, y, yaw, currenttime;
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
		if(current->lowerFrequencyBound == lowerFFTBounds[whichbin])
		{
			//if existing bin is found then stop searching.
			break;
		}
		last = current;
		current = current->next;
	}

	//if there is no audio bin for the sound then make one
	if(current == NULL)
	{
		current = new AudioBin(lowerFFTBounds[whichbin], last, NULL);
		numberOfBins++;
		//if this is the first bin in the LL then point the Handler to it
		if(environment == NULL)
		{
			environment = current;
			current->previous = NULL;
		}
		else last->next = current;
	}

	//add data to the audio bin entry
	simProxy->GetPose2d(robotName, x, y, yaw);

	//todo fix the next lines when clock stuff is sorted.
	//simProxy->GetProperty(name, timeflag, &currenttime, sizeof(currenttime));
	currenttime = (double)time(NULL);
	current->addTone(x, y, currenttime+(duration/1000));



	//if so, append this tone to the end of the tone list and update the list.
	//if not, create a new audio bin and populate it with data from the epuck


	return;
}

int AudioHandler::numberOfTones(void)
{
	return numberOfBins;
}

int AudioHandler::getTones(char* robotName, audio_message_t *store, size_t storesize)
{
	//find how many audio_message_t slots have been allocated and see if it is enough
	int i, numberAllocatedSlots;
	AudioBin *binptr = environment;
	audio_message_t *message = store;

	numberAllocatedSlots = storesize/sizeof(audio_message_t);

	if(numberOfBins > numberAllocatedSlots)
	{
		printf("There are %d tones in the environment, but you have only reserved enough space for %d. Try again\n", numberOfBins, numberAllocatedSlots);
		return 1;
	}

	//get positional info about the robot calling this function
	double x, y, yaw;
	simProxy->GetPose2d(robotName, x, y, yaw);

	//for each bin...
	while(binptr != NULL)
	{
		//find distance between
	}


}




void AudioHandler::dumpData_TEST(void)
{
	AudioBin *binptr = environment;
	AudioBin::audio_tone_t *toneptr;

	printf("AudioHandler has %d bins containing stored data:\n", numberOfBins);
	if(binptr == NULL)
	{
		printf("\tno audio data\n");
		return;
	}

	while(binptr != NULL)
	{
		printf("\nBin lower bound is %f\n", binptr->lowerFrequencyBound);
		printf("\tapparent x is %f\n", binptr->x);
		printf("\tapparent y is %f\n", binptr->y);
		printf("Stored tones:\n");
		toneptr = binptr->tones;
		while(toneptr != NULL)
		{
			printf("\t\tx: %f, y: %f, end: %f\n", toneptr->tx, toneptr->ty, (double)toneptr->end);
			toneptr = toneptr->next;
		}
		binptr = binptr->next;
	}

	return;
}

//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================



int AudioHandler::removeBin(AudioBin *del)
{
	//Four possibilities for this, each needing different stuff doing.
	//case 1, tone is the first in the list.
		//case 1a tone is the only thing in the list
	//case 2, tone is in the middle
	//case 3 tone is at the end of the list

	printf("deleting a bin %f\n", del->lowerFrequencyBound);

	//if next is not null then update its previous pointer to this.previous
	if(del->next != NULL) del->next->previous = del->previous;

	//if previous is not null update its next pointer to this.next
	if(del->previous != NULL) del->previous->next = del->next;

	//is this the first tone?
	if(del->previous == NULL)
	{
		//is this the only tone?
		if(del->next == NULL)
		{
			//delete tone and flag as empty.
			delete del;
			numberOfBins = 0;
			return 1;
		}

		//update tones to point to first item in list
		environment = del->next;
	}
	delete del;
	numberOfBins--;
	return 0;
}

void AudioHandler::updateAudioBinListThreaded(void)
{
	printf("AudioHandler is threaded\n");
	AudioBin *ptr = environment;
	char simproxFlag[] = "sim_time";

	while(true)
	{
		ptr = environment;
		double currentTime;

		//todo fix next lines when stage is updated
		//simProxy->GetProperty(aRobotName, simproxFlag, currentTime, sizeof(currenttime));
		currentTime = (double)time(NULL);
		while(ptr != NULL)
		{
			//updateList(currentTime) returns 1 if list is now empty
			if(ptr->updateList(currentTime))
			{
				//delete current bin
				AudioBin *del = ptr;
				//move ptr along one so that the place in the list isn't lost.
				ptr = ptr->next;

				//removeBin returns 1 if there are no more audiobins
				if(removeBin(del))
				{
					environment = NULL;
					printf("no more bins\n");
				}

				//already advanced the ptr so should skip rest of loop
				continue;
			}
			ptr = ptr->next;
		}
		//sleep for 50ms. This is a lot but the sim only goes to a resolution of 100ms
		usleep(50000);
	}
	pthread_exit(NULL);
	return;
}



