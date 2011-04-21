#include <stdio.h>
#include "libplayerc++/playerc++.h"

using namespace PlayerCc;

int main(void)
{
	PlayerClient epuck("localhost", 6665);
	Position2dProxy p2dProxy(&epuck, 0);
	SonarProxy sonarProxy(&epuck, 0);
	
	while(true)
	{
		double left, right;
		epuck.Read();
		
		left = sonarProxy.GetScan(0);
		right = sonarProxy.GetScan(1);
		
		printf("left sonar: %f, right sonar: %f\n", left, right);
		
		usleep(250000);	
	}
	return 0;
}
