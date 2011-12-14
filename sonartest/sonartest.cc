#include <stdio.h>
#include "libplayerc++/playerc++.h"

using namespace PlayerCc;

int main(void)
{
	PlayerClient epuck("localhost", 6665);	
	RangerProxy rProxy(&epuck, 0);
	printf("got here1?\n");
	Position2dProxy p2dProxy(&epuck, 0);
	printf("got here2?\n");
	
	double left, right;
	double turn=0;
	
	while(true)
	{
		
		epuck.Read();
		printf("got here3?\n");
		
		left	= rProxy.GetRange(1);
		printf("got here4?\n");
		right	= rProxy.GetRange(0);
		printf("got here5?\n");
		
		printf("left sonar: %f, right sonar: %f\n", left, right);
		
		turn += 0.5;
		p2dProxy.SetSpeed(1, turn);
		usleep(500000);	
	}
	return 0;
}
