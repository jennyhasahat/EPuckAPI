/*
 * AudioBin.cc
 *
 *  Created on: 27 Sep 2010
 *      Author: Jennifer Owen
 */

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
void AudioHandler::AudioBin::addTone(double x, double y, double voltage, double endtime)
{
	//construct new audiotone from the supplied data
	audio_tone_t* newtone = new audio_tone_t;

	newtone->tx = x;
	newtone->ty = y;
	newtone->wattsAtSource = (voltage * voltage)/(EPuck::IMPEDANCE_OF_SPEAKER_OHMS);
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

	return;
}

/**
 * Given the position and yaw of the robot this function calculates the cumulative level and direction of the tones in this frequency bin.
 * This is written to an audio_message_t data structure, the pointer to which the calling function must supply.
 * @param xr the x position of the robot
 * @param yr the y position of the robot
 * @param yaw the yaw of the robot. In radians because that's the measure used by playerstage
 * @param output the audio_message_t pointer where the data should be stored.
 * */
void AudioHandler::AudioBin::calculateCumulativeDataForPosition(double xr, double yr, double yaw, audio_message_t* output)
{
	audio_tone_t *ptr =  tones;
	double meanPolarX = 0;
	double meanPolarY = 0;
	int numTones = 0;

	//reset any values in the memory
	output->direction 	= 0;
	output->volume 		= 0;
	output->frequency 	= lowerFrequencyBound;

	//for each tone in this bin.
	while(ptr != NULL)
	{
		double xdiff, ydiff, dist;
		double toneVol;
		int toneDirection;

		//calculate the level and direction of the tone
		//first work out the distance from the source to the robot
		xdiff 	= ptr->tx - xr;
		ydiff 	= ptr->ty - yr;
		dist 	= sqrt( (xdiff * xdiff) + (ydiff * ydiff) );

		toneVol = getSoundIntensity(ptr->wattsAtSource, dist);
		toneDirection = convertDifferentialCoordsIntoBearing(xdiff, ydiff, yaw);

		//if tone is louder it has more of an effect on the tone direction than if it does not.
		//the direction and the volume create a set of polar coordinates which we need to average to get the most accurate direction.
		//using the average of ALL polar coordinates that we worked out.

		meanPolarX += toneVol * cos( degreesToRadians(toneDirection) );
		meanPolarY += toneVol * sin( degreesToRadians(toneDirection) );
		numTones++;

		ptr = ptr->next;
	}

	//now we have the total x and y contributions of each tone we find the average
	meanPolarX = meanPolarX / numTones;
	meanPolarY = meanPolarY / numTones;

	//and convert back to polar coords.
	output->volume = sqrt( (meanPolarX * meanPolarX) + (meanPolarY * meanPolarY) );
	output->direction = convertDifferentialCoordsIntoBearing(meanPolarX, meanPolarY, 0);

	//printf("\tbin direction is %d, volume is %f\n", output->direction, output->volume);

	return;
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
	return 0;
}

/**
 * Function to convert a distance in metres into a volume for the robots.
 * @param levelAtSource the number of watts produced at by the speaker at the tone source.
 * @param distance the distance in metres
 * @returns intensity the sound intensity of the tone in W/m^2. This is then used in {@link AudioBin#addTwoSoundIntensities }
 * */
double AudioHandler::AudioBin::getSoundIntensity(double levelAtSource, double distance)
{
	double area;

	//if distance = 0 then this is the robot making the noise
	if(distance <= 0) return levelAtSource;

	//the sound makes a hemisphere of radius r (where r is distance from source to destination)
	//the sound is spread evenly over the surface of this hemisphere
	//so divide original sound level by surface area of hemisphere
	//(add 1 to hemisphere area to get rid of innaccuracies when area < 1.)

	//area of a hemisphere = 2 pi r^2
	area = 2 * PI * distance * distance;

	return levelAtSource/(1+area);
}

/**
 * Adds one sound intensity level to another and gives the resulting sound intensity level.
 * All measures are assumed to be in W/m^2. I am not an acoustician this means nothing to me.
 * Numbers go in, number come out, what more do I need to know? I found the formula at: <br>
 * http://www.suite101.com/content/how-loud-is-it-a62825
 * @param sound1 level of a sound in W/m^2.
 * @param sound2 level of a sound in W/m^2.
 * @returns soundSum combined level of the two sounds in dB SPL.
 * */
double AudioHandler::AudioBin::addTwoSoundIntensities(double sound1, double sound2)
{
	printf("sound1 is %f, sound2 is %f.\n", sound1, sound2);
	//if either entry is 0 then don't bother with calculations
	if(sound1 == 0) return sound2;
	if(sound2 == 0) return sound1;

	double f1, f2, out;

	f1 = pow(10, sound1/10);
	f2 = pow(10, sound2/10);

	out = log10(f1 + f2);
	printf("\tf1 is %f, f2 is %f\n", f1, f2);
	printf("\tout is %f returned is %f\n", out, 10*out);

	return 10*out;
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

	//gets rid of divide by 0 troubles
	if( (xdiff ==0) && (ydiff == 0) ) return 0;

	//first calculate bearing wrt the x axis
	bearingWRTx =  radiansToDegrees(atan(ydiff/xdiff));
	//if source is to left of receiver, bearing is out by 180deg.
	//adding 360 otherwise gets rid of any negative tan results.
	if(xdiff < 0) 	bearingWRTx += 180;
	else			bearingWRTx += 360;
	bearingWRTx = bearingWRTx%360;

	// did some maths to work out the next line. It works for all orientations of the sender and receiver.
	// This is becuase bearing wrt x and yaw have been changed to be in range 0 to 360.
	bearingWRTrobot = 360 - yaw + bearingWRTx;
	bearingWRTrobot = bearingWRTrobot%360;

	return (int)roundToNearest(bearingWRTrobot, 5);
}



/**
 * converts radians to degrees.
 * */
int AudioHandler::AudioBin::radiansToDegrees(double rads)
{
	double degs;

	degs = 180*rads;
	degs = degs/PI;

	//printf("%f radians is %d degrees\n", rads, (int)degs);

	return (int)degs;
}

/**
 * converts degrees to radians
 * */
double AudioHandler::AudioBin::degreesToRadians(int degs)
{
	double rads;

	rads = PI*degs;
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
 * @returns out the answer.
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

