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
	testbot1->playTone(520, 2500, 5);
	testbot2->playTone(530, 100, 50);
	testbot3->playTone(530, 1500, 4);
	testbot4->playTone(530, 1500, 4);

	testbot1->setAllLEDSOn();
	int i = testbot1->listenForTones();
	testbot1->getTone(i);

	printf("sleeping\n");
	//sleep(5);
	//testbot1->getSimulationTime();


	testbot1->playTone(500, 250, 0.5);



	printf("test finished now\n");
	return 0;
}
