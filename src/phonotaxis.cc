/*
 * Code to make one robot move towards another robot using sound.
 *
 *  Created on: 21 Oct 2010
 *      Author: Jennifer Owen
 */

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include "EPuck.h"


void avoidObjects(EPuck *bot, double *leftWheel, double *rightWheel)
{
	double leftIR, rightIR;
	double left, right;
	const double tooClose = 0.04;

	//check ir sensors left
	leftIR  = bot->getIRReading(2);
	//check IR sensors right
	rightIR = bot->getIRReading(7);

	printf("left IR reads: %f, right IR reads: %f\n", leftIR, rightIR);

	if( (leftIR > tooClose) && (rightIR > tooClose) )
	{
		//if nothing is too close then do nothing
		return;
	}
	else if(leftIR < tooClose)
	{
		//if there's an object to the left move right
		left = EPuck::MAX_WHEEL_SPEED;
		left = left/2;
		right = EPuck::MAX_WHEEL_SPEED;
		right = -right/2;

	}
	else if(rightIR < tooClose)
	{
		//if there's an object to the right move left
		left = EPuck::MAX_WHEEL_SPEED;
		left = -left/2;
		right = EPuck::MAX_WHEEL_SPEED;
		right = right/2;
	}
	else
	{
		//if there;s objects in both of them move back
		left = EPuck::MAX_WHEEL_SPEED;
		left = -left/2;
		right = EPuck::MAX_WHEEL_SPEED;
		right = -right/2;
	}

	*leftWheel = left;
	*rightWheel = right;

	return;
}

/**
 * Make a robot move towards a sound
 * @param bot the robot we want to move towards the sound
 * @param leftWheel the int to write a left wheel speed to
 * @param rightWheel the int to write a right wheel speed to
 * */
void phonotaxis(EPuck *bot, double *leftWheel, double *rightWheel)
{
	int numberTones;
	double left, right;

	numberTones = bot->listenForTones();

	if(numberTones > 0)
	{
		double rads;
		EPuck::Tone t;

		t = bot->getTone(0);
		printf("detected a sound of frequency %f, volume %f and bearing %d\n", t.frequency, t.volume, t.bearing);

		//bearing is in range 0 to 360
		//the close to 0 or 360 the number is the more similar we want the wheel speeds to be
		//cos returns number that is scaled between -1 and 1 depending on the bearing with 180 being -1 and 0 or 360 being +1

		rads = (t.bearing*3.14159)/180;

		//if the tone is coming from the left side of the robot...
		if( t.bearing < 180)
		{
			//slow left wheel
			right = 1;
			left  = cos(rads);
		}
		else
		{
			//slow right wheel
			left  = 1;
			right = cos(rads);
		}
		printf("\tphonotaxis: left %f, right %f\n", left, right);

		//then scale results so the speeds are between -0.04 and +0.04
		right *= EPuck::MAX_WHEEL_SPEED;
		left  *= EPuck::MAX_WHEEL_SPEED;
	}
	else
	{
		//random wandering
		left = rand() % 10; //num between 0 and 9
		left = EPuck::MAX_WHEEL_SPEED*(left / 1000);	//num between 0 and max speed to 1dp
		right = rand() % 10; //num between 0 and 9
		right = EPuck::MAX_WHEEL_SPEED*(right / 1000);	//num between 0 and max speed to 1dp
		left += *leftWheel;
		right += *rightWheel;

		printf("random walk: left %f, right %f\n", left, right);
	}

	//copy info into provided memory slot
	*leftWheel = left;
	*rightWheel = right;

	return;
}


/**
 * Function to make a robot's presence known.
 * @param the robot to flash and make a sound (cast to a void*)
 * */
void *flashAndSound(void *bot)
{
	EPuck *robot = (EPuck*)bot;
	robot->setAllLEDsOn();
	robot->printLocation_TEST();
	while(true)
	{
		robot->playTone(500, 500, 5);
		usleep(500000);
	}
	pthread_exit(NULL);
	return NULL;
}


int main(void)
{
	EPuck* robots[4];
	pthread_t noisyBotThread;

	char testbot1Name[] = "robot1";
	robots[1] = new EPuck(6665, testbot1Name);
	robots[1]->initaliseAudio();

	char testbot2Name[] = "robot2";
	robots[2] = new EPuck(6666, testbot2Name);
	robots[2]->initaliseAudio();

	char testbot3Name[] = "robot3";
	robots[3] = new EPuck(6667, testbot3Name);
	robots[3]->initaliseAudio();

	char testbot4Name[] = "robot4";
	robots[4] = new EPuck(6668, testbot4Name);
	robots[4]->initaliseAudio();

	int bot;
	char noisebot[] = "robot%d";

	printf("Which robot would you like to make a noise (enter 1, 2 or 3): ");
	scanf("%d", &bot);
	while(bot < 1 || bot > 3)
	{
		printf("\nWhich robot would you like to make a noise (enter 1, 2 or 3): ");
		scanf("%d", &bot);
	}
	printf("\nYou have chosen robot %d. It will flash to show which one it is.\n", bot);
	bot++;
	sprintf(noisebot, noisebot, bot);

	//set the bot flashing and noising
	pthread_create(&noisyBotThread, NULL, flashAndSound, (void *)robots[bot]);

	double left, right;

	while(true)
	{
		phonotaxis(robots[1], &left, &right);
		avoidObjects(robots[1], &left, &right);

		robots[1]->setDifferentialMotors(left, right);
		usleep(50000);
	}

	return 0;
}
