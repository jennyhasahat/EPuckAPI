/*
 * AudioHandler.cc
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#include "EPuck.h"
#include "AudioHandler.h"




//I hope this statement doesn't have global scope... I don't think it does.
AudioHandler* AudioHandler::_instance = 0;


/**
 * Creates the audiohandler object and builds an array to store the sound data for each robot.
 * @param simulationClient the PlayerClient object which handles the simulation.
 * @param sim the simulationProxy attached to the playerclient handling this simulation.
 * @param name the name of the robot that initialises the AudioHandler.
 * This is used for accessing data from the simulation proxy, as you need the name of a model to get simulation time information.
 * */
AudioHandler::AudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name)
{
	int i;

	simClient = simulationClient;
	simProxy = sim;
	environment = NULL;
	strncpy(aRobotName, name, 32);
	numberOfBins = 0;

	//make array of frequency bins depending on FFT settings.
	for(i=0; i<fftBlockSize/2; i++)
	{
		lowerFFTBounds[i] = (i*sampleRate)/fftBlockSize;
	}

	updateAudioBinListThread = boost::thread(&AudioHandler::updateAudioBinListThreaded, this);
	//pthread_create(&updateAudioBinListThread, 0, AudioHandler::startupdateAudioBinListThread, this);

	printf("AudioHandler initialised\n");
	return;
}

/**
 * Function that allows AudioHandler to be a singleton. Use this to construct the AudioHandler object.
 * @param simulationClient the PlayerClient object which handles the simulation.
 * @param sim the simulationProxy attached to the playerclient handling this simulation.
 * @param name the name of the robot that initialises the AudioHandler.
 * This is used for accessing data from the simulation proxy, as you need the name of a model to get simulation time information.
 * */
AudioHandler* AudioHandler::GetAudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name)
{
	if(_instance == 0)
	{
		_instance = new AudioHandler(simulationClient, sim, name);
	}
	return _instance;
}

AudioHandler::~AudioHandler()
{
	//close thread
	updateAudioBinListThread.interrupt();
	updateAudioBinListThread.join();

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

/**
 * Puts a tone of the desired frequency and duration into the audio environment.
 * @param freq the frequency of the tone in Hz
 * @param duration the length of the tone in milliseconds
 * @param volume how loud to play the tone at. A number between 0 and 10. This does not go up to 11.
 * @param robotName string containing the name of the robot playing the tone.
 * */
void AudioHandler::playTone(int freq, double duration, double volume, char* robotName)
{
	int whichbin;
	char timeflag[] = "time";
	const int maxVoltage = EPuck::MAXIMUM_BATTERY_VOLTAGE;
	const int minVoltage = 0; //EPuck::MINIMUM_BATTERY_VOLTAGE;

	double x, y, yaw, currenttime, voltage;
	AudioBin *current = environment;
	AudioBin *last;

	//find FFT lower frequency bound for freq
	for(whichbin=(fftBlockSize/2)-1; whichbin>-1; whichbin--)
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

	//get xy coords.
	simProxy->GetPose2d(robotName, x, y, yaw);

	//get simulation time
	simProxy->GetProperty(robotName, timeflag, &currenttime, sizeof(currenttime));
	//currenttime = (double)time(NULL);

	//limit voltage to be between the max and min voltages.

	//find how much voltage is represented by each "volume" unit.
	voltage = maxVoltage - minVoltage;	//what's our working voltage range
	voltage = voltage/10;	//there are 10 units so divide by 10 to get volts per unit
	voltage *= volume;		//scale volts per unit by number of "volume units"
	voltage += minVoltage;	//in case minVoltage is non-zero

	//incase the requested volume is less than 0 or more than 10
	if(voltage > maxVoltage) voltage = maxVoltage;
	else if(voltage < minVoltage) voltage = minVoltage;

	current->addTone(x, y, voltage, currenttime+(duration/1000));

	return;
}

/**
 * Returns the number of AudioBins currently in the environment. This function is needed so that space can be allocated for the Tones in the EPuck code.
 * @returns notones the number of different frequency tones the robot can detect.
 * */
int AudioHandler::getNumberOfTones(void)
{
	return numberOfBins;
}

/**
 * Provides the audio data in the environment, including frequency, volume and direction of the tones.
 * The function which calls this must allocate the memory needed as an array, and provide the address and size of the memory slot.
 * This function will then copy the environmental audio data into the provided memory using the audio_message_t structure.
 * @param robotName	the name of the robot which is requesting the data (this is given in the worldfile)
 * @param store		link to where the audio data memory has been allocated.
 * @param storesize	the number of audio_message_t objects allocated to the audio data.
 * @returns the number of tones in the environment that this robot can detect, -1 if unsuccessful (like say if new data has been added between allocating memory and trying to copy it over).
 * @see AudioHandler#getNumberOfTones()
 *
 * */
int AudioHandler::getTones(char* robotName, audio_message_t *store, size_t storesize)
{
	int numberAllocatedSlots;
	double x, y, yaw;
	int i = 0;
	AudioBin *binptr = environment;

	//find how many audio_message_t slots have been allocated and see if it is enough
	numberAllocatedSlots = storesize/sizeof(audio_message_t);

	if(numberOfBins > numberAllocatedSlots)
	{
		printf("There are %d tones in the environment, but you have only reserved enough space for %d. Try again.\n", numberOfBins, numberAllocatedSlots);
		return -1;
	}

	//get positional info about the robot calling this function
	simProxy->GetPose2d(robotName, x, y, yaw);

	//for each bin get the full tone information for it.
	while(binptr != NULL && i < numberAllocatedSlots)
	{
		//printf("looking at bin %f, ", binptr->lowerFrequencyBound);
		//printf("putting this data in index %d\n", i);
		binptr->calculateCumulativeDataForPosition(x, y, yaw, &store[i]);
		i++;
		binptr = binptr->next;
	}

	// the epucks have a hearing range of AT MOST 10cm on a good day.
	//deal with this here

	return 0;
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
		printf("Stored tones:\n");
		toneptr = binptr->tones;
		while(toneptr != NULL)
		{
			printf("\t\tx: %f, y: %f, wattage: %f, end: %f\n", toneptr->tx, toneptr->ty, toneptr->wattsAtSource, (double)toneptr->end);
			toneptr = toneptr->next;
		}
		binptr = binptr->next;
	}

	return;
}


//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================


/**
 * Removes an AudioBin from the Linked list.
 * @param del AudioBin to remove from LL.
 * */
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

/**
 * Updates the list of AudioBins so that tones which have finished playing are removed from data storage.
 * This function is intended to be threaded, so contains a loop which does not terminate.
 * */
void AudioHandler::updateAudioBinListThreaded(void)
{
	printf("AudioHandler is threaded\n");
	boost::posix_time::milliseconds wait(50);
	AudioBin *ptr = environment;
	char simproxFlag[] = "time";

	while(true)
	{
		ptr = environment;
		double currentTime;

		//todo fix next lines when stage is updated
		simProxy->GetProperty(aRobotName, simproxFlag, &currentTime, sizeof(currentTime));
		//currentTime = (double)time(NULL);
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
				}

				//already advanced the ptr so should skip rest of loop
				continue;
			}
			ptr = ptr->next;
		}
		//sleep for 50ms. This is a lot but the sim only goes to a resolution of 100ms
		boost::this_thread::sleep(wait);
	}

	return;
}



