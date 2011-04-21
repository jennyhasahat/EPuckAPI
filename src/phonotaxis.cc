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

/**
 * Checks the given robot for nearby obstacles and generates motor speeds to avoid them.
 * If no obstacles are generated, no new motor speeds are generated.
 * The leftwheel and rightwheel parameters are unchanged.
 * @param bot the robot to avoid obstacles with
 * @param leftwheel the double to write the new wheel speed to
 * @param rightwheel the double to write the new wheel speed to
 * @return true if avoiding an obstacle, false otherwise.
 * */
bool avoidObjects(EPuck *bot, double *leftWheel, double *rightWheel)
{
	double leftIR, rightIR;
	double left, right;
	const double tooClose = 0.04;

	//check ir sensors left
	leftIR  = bot->getIRReading(2);
	//check IR sensors right
	rightIR = bot->getIRReading(7);

	if( (leftIR > tooClose) && (rightIR > tooClose) )
	{
		//if nothing is too close then do nothing
		return false;
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

	return true;
}

/**
 * Generates random walk wheel speeds.
 * This function counts how many times it is called. When it is called 50 times whilst going forwards it will start to turn.
 * When it is called 20 times whilst turning it will start going forward.
 * @param bot pointer to the epuck we want to random walk
 * @param leftWheel pointer to the left wheel speed
 * @param rightWheel pointer to the right wheel speed
 * */
void randomWalk(EPuck *bot, double *leftWheel, double *rightWheel)
{
	const int forwardCount = 50;
	const int turnCount = 20;

	static int statusCounter = 0;
	static bool isTurning = false;
	static double left = EPuck::MAX_WHEEL_SPEED;
	static double right = EPuck::MAX_WHEEL_SPEED;

	//count how many times this function is called
	statusCounter++;

	//if robot is going forwards and should turn, generate new turning speeds
	if((statusCounter > forwardCount) && !isTurning)
	{
		//random turn amount
		left = (rand() % 11)-5; //num between -5 and 5
		left = EPuck::MAX_WHEEL_SPEED*(left / 5);	//num between -maxspeed and +maxspeed to 1dp
		right = (rand() % 11)-5; //num between -5 and 5
		right = EPuck::MAX_WHEEL_SPEED*(right / 5);	//num between -maxspeed and +maxspeed to 1dp

		//refresh counter
		statusCounter = 0;
		isTurning = true;
	}
	//otherwise, if the robot is turning and should go forwards. generate new forward speeds
	else if((statusCounter > turnCount) && isTurning)
	{
		left = EPuck::MAX_WHEEL_SPEED *0.8;
		right = EPuck::MAX_WHEEL_SPEED *0.8;

		//refresh counter
		statusCounter = 0;
		isTurning = false;
	}

	//copy info into provided memory slot
	*leftWheel = left;
	*rightWheel = right;

	//printf("random walk setting left %f, right %f\n", *leftWheel, *rightWheel);
	return;
}

/**
 * Make a robot move towards a sound
 * @param bot the robot we want to move towards the sound
 * @param leftWheel the double to write a left wheel speed to
 * @param rightWheel the double to write a right wheel speed to
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

		//if the robot can actually hear the tone
		if(t.volume > 0)
		{
			printf("sound heard!\n");
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

			//then scale results so the speeds are between -0.04 and +0.04
			right *= EPuck::MAX_WHEEL_SPEED;
			left  *= EPuck::MAX_WHEEL_SPEED;

			//copy info into provided memory slot
			*leftWheel = left;
			*rightWheel = right;

			printf("phonotaxis setting left %f, right %f\n", *leftWheel, *rightWheel);
		}
		else randomWalk(bot, leftWheel, rightWheel);
	}
	else
	{
		randomWalk(bot, leftWheel, rightWheel);
	}

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
	while(true)
	{
		printf("playing tone\n");
		robot->playTone(500, 5000, 10);
		robot->dumpAudio_TEST();
		usleep(5000000);
	}
	pthread_exit(NULL);
	return NULL;
}


int main(void)
{
	EPuck* robots[4];
	pthread_t noisyBotThread1;

	char testbot1Name[] = "robot1";
	robots[0] = new EPuck(6665, testbot1Name);
	robots[0]->initaliseAudio();

	char testbot2Name[] = "robot2";
	robots[1] = new EPuck(6666, testbot2Name);
	robots[1]->initaliseAudio();

	char testbot3Name[] = "robot3";
	robots[2] = new EPuck(6667, testbot3Name);
	robots[2]->initaliseAudio();

	char testbot4Name[] = "robot4";
	robots[3] = new EPuck(6668, testbot4Name);
	robots[3]->initaliseAudio();

	int bot1;

	printf("Which robot would you like to make a noise (enter 1, 2 or 3): ");
	scanf("%d", &bot1);
	while(bot1 < 1 || bot1 > 3)
	{
		printf("\nWhich robot would you like to make a noise (enter 1, 2 or 3): ");
		scanf("%d", &bot1);
	}

	//set the bot flashing and noising
	pthread_create(&noisyBotThread1, NULL, flashAndSound, (void *)robots[bot1]);
	usleep(100000);

	double left, right;

	while(true)
	{
		phonotaxis(robots[0], &left, &right);

		if(avoidObjects(robots[0], &left, &right))
		{
			printf("avoiding obstacles\n");
		}

		robots[0]->setDifferentialMotors(left, right);
	//	printf("left: %f. right %f\n", left, right);
		usleep(50000);
	}

	return 0;
}
