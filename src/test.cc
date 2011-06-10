#include <stdio.h>
#include "EPuck.h"

/**
Test program for EPuck class.
Runs through functions in EPuck class and prints responses to standard output.
*/
int main(void)
{
	//initialise robots

	EPuck *testbot1;
	char testbot1Name[] = "robot1";
	testbot1 = new EPuck(6665, testbot1Name);

	EPuck *testbot2;
	char testbot2Name[] = "robot2";
	testbot2 = new EPuck(6666, testbot2Name);

	EPuck *testbot3;
	char testbot3Name[] = "robot3";
	testbot3 = new EPuck(6667, testbot3Name);
	
	EPuck *testbot4;
	char testbot4Name[] = "robot4";
	testbot4 = new EPuck(6668, testbot4Name);


	//do something with the robots
	while(1)
	{
		testbot1->setDifferentialMotors(1, 1);
		testbot2->setDifferentialMotors(2, 0);
		testbot3->setDifferentialMotors(0, 2);
	}


	return 0;
}
