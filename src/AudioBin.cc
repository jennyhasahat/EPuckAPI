#include "EPuck.h"
#include "AudioHandler.h"

/**
 * Creates a new AudioBin item. The AudioBin linked list uses the lower frequency bound as a key, so this must be given
 * upon initialisation.
 * @param lowerFreq the lower frequency bound of this audio bin.
 * @param prev the previous AudioBin in the Linked list the bins are stored in.
 * @param nxt the next AudioBin in the Linked list the bins are stored in.
 * */
AudioHandler::AudioBin::AudioBin(double lowerFreq, AudioBin *prev, AudioBin *nxt)
{
	lowerFrequencyBound = lowerFreq;
	next = nxt;
	previous = prev;
	tones = NULL;
	numberTones = 0;

	return;
}

AudioHandler::AudioBin::~AudioBin()
{
	//remove all tones in Linked list
	audio_tone_t *ptr = tones;
	audio_tone_t *prev;

	while(ptr != NULL)
	{
		prev = ptr;
		ptr = ptr->next;
		delete prev;
	}

	return;
}

/**Returns the number of tones currently stored in this audio bin*/
int AudioHandler::AudioBin::getNumberTones(void)
{
	return numberTones;
}

/**Updates list of tones in the bin so that ones which have finished playing are removed.
 * Also updates apparent sound sources
 * @param currentTime the current time of the simulation
 * @returns status 0 if a tone is removed but there are still tones in the AudioBin, 1 if entire list is now empty.*/
int AudioHandler::AudioBin::updateList(double currentTime)
{
	//move through LL and remove any entries with end times after the current time
	audio_tone_t *ptr;

	ptr = tones;
	while(ptr != NULL)
	{
		audio_tone_t *del = ptr;

		ptr = ptr->next;
		//if tone should have finished
		if(del->end <= currentTime)
		{
			//delete it and if LL is empty now return 1
			if(removeTone(del)) return 1;
		}
	}
	return 0;
}

/**
 * Adds an audio_tone_t into the AudioBin. Creates an audio_tone_t object, appends it to the linked list and updates itself.
 * @param x the x position of the robot playing the tone
 * @param y the y position of the robot playing the tone
 * @param voltage how loud to play the tone at. Value indicates in VOLTS how big the signal to the speaker should be.
 * This must be in the range 0 to 5, if this number is outside the range it will be assumed to be either 0 or 5 (whichever is closer).
 * So if you set this as 3, the speaker will play a sine wave at 3 volts (peak to peak).
 * @param endtime the simulated time at which the tone will end.
 * */
void AudioHandler::AudioBin::addTone(double x, double y, double endtime)
{
	//construct new audiotone from the supplied data
	audio_tone_t* newtone = new audio_tone_t;

	newtone->tx = x;
	newtone->ty = y;
	//newtone->wattsAtSource = (voltage * voltage)/(EPuck::IMPEDANCE_OF_SPEAKER_OHMS);
	newtone->end = endtime;
	newtone->next = NULL;

	//if there is no data in linked list then make this the first entry.
	if(tones == NULL)
	{
		tones = newtone;
		tones->previous = NULL;
	}
	else
	{
		//search linked list until the end is reached
		audio_tone_t *toneptr = tones;

		while(toneptr->next != NULL)
		{
			toneptr = toneptr->next;
		}

		//append newtone to LL
		newtone->previous = toneptr;
		toneptr->next = newtone;
	}
	numberTones++;

	return;
}

/**
 * Given the position and yaw of the listening robot this function calculates the distance and direction
 * of each tone in this frequency bin and saves it to the provided audio_message_t array.
 * The user must allocate the memory for the audio message array in a higher function.
 * @param xr the x position of the robot
 * @param yr the y position of the robot
 * @param yaw the yaw of the robot. In radians because that's the measure used by playerstage
 * @param output the audio_message_t pointer where the data should be stored.
 * @param noMsgs the number of audio messages to retrieve from the bin
 * @returns the number of tones actually saved by this function, so 0 if there were none.
 * */
int AudioHandler::AudioBin::calculateRawToneDataForPosition(double xr, double yr, double yaw, audio_message_t* output, int noMsgs)
{
	audio_tone_t *ptr =  tones;
	int currentTone = 0;

	//for each tone in this bin.
	for(currentTone=0; currentTone<noMsgs; currentTone++)
	{
		double xdiff, ydiff;

		//calculate the x y distances to the tone from the listening robot
		xdiff 	= ptr->tx - xr;
		ydiff 	= ptr->ty - yr;

		output[currentTone].distance	= sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		output[currentTone].direction	= convertDifferentialCoordsIntoBearing(xdiff, ydiff, yaw);
		output[currentTone].frequency 	= lowerFrequencyBound;

		//advance place in linked list
		ptr = ptr->next;
		//if we reach the end of the linked list before running out of storage space then break the loop
		if(ptr == NULL) break;
	}

	return currentTone+1;
}





//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================

/**
 * Removes the supplied audioTone from the linked list.
 * @param *del address of the tone to delete.
 * @returns success 0 if tone is deleted, 1 if tone is deleted and the bin is now empty.
 * */
int AudioHandler::AudioBin::removeTone(audio_tone_t *del)
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
		tones = del->next;

		//is this the only tone?
		if(del->next == NULL)
		{
			//delete tone and flag as empty.
			delete del;
			return 1;
		}
	}
	delete del;
	numberTones--;

	return 0;
}




/**
 * Function to convert two coordinates into a bearing.
 * Takes the coordinates of the sound source and the sound reciever, and works out the bearing of the sound source wrt the reciever.
 * Bearings are measured anticlockwise from where the robot is facing straight ahead, the unit used is degrees (not radians).
 * @param xdiff the source x coordinate minus the reciever x coordinate
 * @param ydiff the source y coordinate minus the reciever y coordinate
 * @param recieverYaw the yaw of the robot that is listening for tones.
 * */
int AudioHandler::AudioBin::convertDifferentialCoordsIntoBearing(double xdiff, double ydiff, double recieverYaw)
{
	int yaw, bearingWRTx, bearingWRTrobot;

	//convert yaw to degrees
	yaw = radiansToDegrees(recieverYaw);
	//convert yaw so that it is between 0 and 360
	//puts yaw in range -360 to 360
	yaw = yaw%360;
	//get rid of any pesky minuses by adding 360. Now in range 0 to 720
	yaw += 360;
	//modulo again to get in range 0 to 360
	yaw = yaw%360;

	//get rid of divide by 0 troubles
	if( (xdiff ==0) && (ydiff == 0) ) return 0;

	//first calculate bearing wrt the x axis
	bearingWRTx =  radiansToDegrees(atan(ydiff/xdiff));
	//if source is to left of receiver, bearing is out by 180deg.
	//adding 360 otherwise gets rid of any negative tan results.
	if(xdiff < 0) 	bearingWRTx += 180;
	else			bearingWRTx += 360;
	bearingWRTx = bearingWRTx%360;

	// did some maths to work out the next line. It works for all orientations of the sender and receiver.
	// This is becuase bearing wrt x and yaw have already been changed to be in range 0 to 360.
	bearingWRTrobot = 360 - yaw + bearingWRTx;
	bearingWRTrobot = bearingWRTrobot%360;

	//return (int)roundToNearest(bearingWRTrobot, 45);
	return (int)bearingWRTrobot;
}



/**
 * converts radians to degrees.
 * */
int AudioHandler::AudioBin::radiansToDegrees(double rads)
{
	double degs;

	degs = 180*rads;
	degs = degs/3.141592;

	//printf("%f radians is %d degrees\n", rads, (int)degs);

	return (int)degs;
}

/**
 * converts degrees to radians
 * */
double AudioHandler::AudioBin::degreesToRadians(int degs)
{
	double rads;

	rads = degs*3.141592;
	rads = rads/180;

	//printf("%d degrees is %f radians\n", degs, rads);

	return rads;
}

/**
 * Rounds the given number to the nearest multiple of the "resolution" parameter.
 * e.g. if input is 23.720954 and resolution is 5 this will return 25.
 * e.g. if input is 23.720954 and resolution is 0.5 this will return 23.5.
 * @param input the number to round
 * @param resolution the resolution to round to
 * @returns the rounded number
 * */
double AudioHandler::AudioBin::roundToNearest(double input, double resolution)
{
	double floor, ceiling;
	int temp;

	//using an int here because the type cast automatically rounds DOWN to the nearest whole integer. How useful!
	temp = input/resolution;
	floor = temp*resolution;

	temp = (input+resolution)/resolution;
	ceiling = temp*resolution;

	//which is it closer to?
	//if floor is further away that ceil. return ceil.
	if( (input - floor) > (ceiling - input) )
	{
		return ceiling;
	}
	else return floor;
}

