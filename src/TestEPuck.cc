#include <stdio.h>
#include "EPuck.h"

/**
Test program for EPuck class.
Runs through functions in EPuck class and prints responses to standard output.
*/
int main(void)
{



	EPuck *testbot1;
	char testbot1Name[] = "robot1";
	testbot1 = new EPuck(6665, testbot1Name);
	testbot1->initaliseAudio();

	EPuck *testbot2;
	char testbot2Name[] = "robot2";
	testbot2 = new EPuck(6666, testbot2Name);
	testbot2->initaliseAudio();

	EPuck *testbot3;
	char testbot3Name[] = "robot3";
	testbot3 = new EPuck(6667, testbot3Name);
	testbot3->initaliseAudio();
	
	EPuck *testbot4;
	char testbot4Name[] = "robot4";
	testbot4 = new EPuck(6668, testbot4Name);
	testbot4->initaliseAudio();



printf("playing tones\n");
//printf("\t 1, ");
	//testbot1->playTone(520, 2500, 5);
printf("\t 2, ");
	testbot2->playTone(530, 100, 5);
printf("\t 3, ");
	testbot3->playTone(730, 1500, 5);
printf("\t 4.\n");
	testbot4->playTone(530, 1500, 5);

	testbot1->setAllLEDsOn();
	printf("listening.\n");
	int numTones = testbot1->listenForTones();
	int i;

	printf("1. in TestEPuck robot heard %d tone(s).\n", numTones);
	for(i=0; i<numTones; i++)
	{
		EPuck::Tone tone = testbot1->getTone(i);
		printf("\tTone %d has freq %f, volume %f and bearing %d\n", i, tone.frequency, tone.volume, tone.bearing);
	}


	testbot3->playTone(1000, 300, 7);

	numTones = testbot1->listenForTones();

	printf("1. in TestEPuck robot heard %d tone(s).\n", numTones);
	for(i=0; i<numTones; i++)
	{
		EPuck::Tone tone = testbot1->getTone(i);
		printf("\tTone %d has freq %f, volume %f and bearing %d\n", i, tone.frequency, tone.volume, tone.bearing);
	}


	printf("sleeping\n");
	//sleep(5);
	//testbot1->getSimulationTime();


	testbot1->playTone(500, 250, 0.5);



	printf("test finished now\n");
	return 0;
}
