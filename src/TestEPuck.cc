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
	char testbot3Name[] = "robot3";
	EPuck *testbot1;
	EPuck *testbot2;
	EPuck *testbot3;
	
	testbot1 = new EPuck(testbot1Name);
	testbot2 = new EPuck(6666, testbot2Name);
	testbot3 = new EPuck(6667, testbot3Name);
	
	testbot1->initaliseAudio();

	testbot2->initaliseAudio();

	testbot3->initaliseAudio();
printf("playing tones\n");
	testbot1->playTone(520, 2500);
	testbot2->dumpAudio_TEST();
	testbot2->playTone(1525, 100);
	testbot2->dumpAudio_TEST();
	testbot3->playTone(530, 1500);
	testbot2->dumpAudio_TEST();

	printf("sleeping\n");
	sleep(5);
	//testbot1->getSimulationTime();


	testbot1->playTone(500, 250);
	testbot2->dumpAudio_TEST();



	printf("test finished now\n");
	return 0;
}
