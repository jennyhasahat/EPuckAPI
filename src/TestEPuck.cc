#include <stdio.h>
#include "EPuck.h"

/**
Test program for EPuck class.
Runs through functions in EPuck class and prints responses to standard output.
*/
int main(void)
{
	char testbot1Name[] = "robot1";
	char testbot2Name[] = "robot2";
	EPuck *testbot1;
	EPuck *testbot2;
	
	testbot1 = new EPuck(testbot1Name);
	testbot2 = new EPuck(6666, testbot2Name);
	
	printf("initialising robot1 audio\n");
	testbot1->initaliseAudio(2);
	printf("initialising robot2 audio\n");
	testbot2->initaliseAudio(2);
	printf("finished initialising audio\n");
	sleep(1);

	//testbot1->playTone(500, 250);
	//testbot2->playTone(400, 250);

/*	while(true)
	{

		testbot1->setDifferentialMotors(0.25, 0.3); //turns right
		testbot2->setDifferentialMotors(0.3, 0.25); //turns left

		printf("testbot 1 going to play a tone f:2000 d:1\n");
		testbot1->playTone(2000, 1);
		printf("tone info sent\n");

		usleep(100);
	}
*/
	return 0;
}
