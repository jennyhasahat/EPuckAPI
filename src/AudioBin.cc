/*
 * AudioBin.cc
 *
 *  Created on: 27 Sep 2010
 *      Author: Jennifer Owen
 */

#include "AudioHandler.h"

/**
		 * Creates a new AudioBin item. The AudioBin linked list uses the lower frequency bound as a key, so this must be given
		 * upon initialisation.
		 * @param lowerfreq the lower frequency bound of this audio bin.
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
	updatePosition();
	return 0;
}

void AudioHandler::AudioBin::addTone(double x, double y, double volume, double endtime)
{
	//construct new audiotone from the supplied data
	audio_tone_t* newtone = new audio_tone_t;

	newtone->tx = x;
	newtone->ty = y;
	newtone->tlevel = volume;
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
	//update the bin's apparent x,y coordinates using the new data.
	updatePosition();

	return;
}

int AudioHandler::AudioBin::calculateCumulativeDataForPosition(double x, double y, double yaw, audio_message_t* output)
{
	audio_tone_t *ptr =  tones;

	//reset any values in the memory
	output->direction 	= 0;
	output->volume 		= 0;
	output->frequency 	= lowerFrequencyBound;

	while(ptr != NULL)
	{
		double xdiff, ydiff, dist;
		double toneDirection, toneVol;

		xdiff 	= ptr->tx - x;
		ydiff 	= ptr->ty - y;
		dist 	= sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		//toneVol = convertDistanceIntoSoundLevel(ptr->tlevel, dist);

		toneDirection = convertDifferentialCoordsIntoBearing(xdiff, ydiff, yaw);

		//if tone is louder it has more of an effect on the tone direction than if it does not.
		//this may actually be a dumb way of doing it because if there's only one tone...
		//TODO this section needs work
		//the direction and the volume create a set of polar coordinates which we need to average to get the most accurate direction.
		ptr = ptr->next;
	}

	return 0;
}



//==================================================================================================
//							PRIVATE FUNCTIONS
//==================================================================================================

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

void AudioHandler::AudioBin::updatePosition(void)
{
	//move through LL and sum all x, and y coordinates. Also count number of entries.
	audio_tone_t *ptr;
	double sumX = 0;
	double sumY = 0;
	int count 	= 0;

	ptr = tones;
	while(ptr != NULL)
	{
		sumX += ptr->tx;
		sumY += ptr->ty;
		count++;
		ptr = ptr->next;
	}
	x = sumX/count;
	y = sumY/count;

	return;
}

double AudioHandler::AudioBin::convertDistanceIntoSoundLevel(double originalLevel, double distance)
{
	double volume;
	const double pi = 3.14159;

	printf("calculating volume:\n\toriginal level is %f, distance is %f\n", originalLevel, distance);

	//if distance = 0 then this is the robot making the noise
	if(distance <= 0) return originalLevel;

	//the sound makes a hemisphere of radius r (where r is distance from source to destination)
	//the sound is spread evenly over all points of this hemisphere
	//so divide original sound level by volume of hemisphere
	//(add 1 to hemisphere vol to get rid of innaccuracies when vol < 1.)

	//Vol of a hemisphere = 2/3 * pi * r^3
	volume = 2*pi*pow(distance, 3)/3;

	printf("\tvolume of hemisphere is %f, returning %f\n", volume, originalLevel/(1+volume) );

	return originalLevel/(1+volume);
}

double AudioHandler::AudioBin::convertDifferentialCoordsIntoBearing(double xdiff, double ydiff, double recieverYaw)
{
	int yaw, bearingWRTx;

	//convert yaw to degrees
	yaw = degreesToRadians(recieverYaw);
	//convert yaw so that it is between 0 and 360
	printf("yaw from p/s is %f, in degrees is %d, yaw%%360 is %d\n", recieverYaw, yaw, (int)yaw%360);
	//puts yaw in range -360 to 360
	yaw = yaw%360;
	//get rid of any pesky minuses by adding 360. Now in range 0 to 720
	yaw += 360;
	//modulo again to get in range 0 to 360
	yaw = yaw%360;


	//first calculate bearing wrt the x axis
	if(xdiff > 0)
	{
		bearingWRTx =  degreesToRadians(tan(xdiff/ydiff));
	}
	else
	{
		bearingWRTx =  degreesToRadians(tan(xdiff/ydiff)+180);
	}
	printf("bearing wrt x is %d. tan(x/y) is %f radians\n", tan(xdiff/ydiff));


	return 200;
}

int AudioHandler::AudioBin::degreesToRadians(double rads)
{
	double degs;

	degs = 180*rads;
	degs = degs/PI;

	return (int)degs;
}

