#include <stdio.h>
#include "../include/EPuck.h"

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
	
	while(true)
	{

		testbot1->setDifferentialMotors(0.25, 0.3); //turns right
		testbot2->setDifferentialMotors(0.3, 0.25); //turns left

		printf("turning on %s leds\n", testbot1Name);
		testbot1->setAllLEDSOn();

		usleep(100);
	}
	
	return 0;
}
