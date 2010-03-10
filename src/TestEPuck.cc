#include <stdio.h>
#include "../include/EPuck.h"

/**
Test program for EPuck class.
Runs through functions in EPuck class and prints responses to standard output.
*/
int main(void)
{
	char testbot1Name[] = "robot1";
	EPuck *testbot1;
	char testbot2Name[] = "robot2";
	EPuck *testbot2;
	
	testbot1 = new EPuck(6665, testbot1Name);
	testbot2 = new EPuck(6666, testbot2Name);
	

	int i=0;
	while(true)
	{

		testbot1->setDifferentialMotors(0.3, 0.25); //turns right
		testbot2->setDifferentialMotors(0.25, 0.3); //turns left

		//if a robot sees something...
		if(testbot1->getNumberBlobs() > 0)
		{
			int noBlobs = testbot1->getNumberBlobs();

			for(i=0; i<noBlobs; i++)
			{
				Blob myblob;
				myblob = testbot1->getBlob(i);
				printf("green robot sees a blob, its colour is %x\n", myblob.colour);
			}
		}

		double *ir = testbot2->getIRReadings();

		printf("red robot's IR readings are:\n");
		printf("\t%1.5f, %1.5f, %1.5f, %1.5f, %1.5f, %1.5f, %1.5f, %1.5f\n", \
				ir[0], ir[1], ir[2], ir[3], ir[4], ir[5], ir[6], ir[7]);


		usleep(100);
	}
	
	return 0;
}
