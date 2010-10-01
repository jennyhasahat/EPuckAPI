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
			printf("\tdeleting a tone x: %f y:%f end: %f current %f\n",
					del->tx, del->ty, del->end, currentTime);
			//delete it and if LL is empty now return 1
			if(removeTone(del)) return 1;
		}
	}
	updatePosition();
	return 0;
}

void AudioHandler::AudioBin::addTone(double x, double y, double endtime)
{
	//construct new audiotone from the supplied data
	audio_tone_t* newtone = new audio_tone_t;

	newtone->tx = x;
	newtone->ty = y;
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
	int count = 0;

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
