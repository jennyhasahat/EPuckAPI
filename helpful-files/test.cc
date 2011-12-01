#include <libplayerc++/playerc++.h>
#include <stdlib.h>
#include <signal.h>
#include "args.h"
#include <iostream>

#define  WHEEL_SEP   0.052

int weightleft[8] = { -10, -10, -5, 0, 0, 5, 10, 10};
int weightright[8] = {10, 10, 5, 0, 0, -5, -10, -10};
int userQuit = 0;
int leftwheel = 0;
int rightwheel = 0;
int irdata[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define BATT_LOG_FILE "./batt.log"


void signalHandler(int dummy);

double getPx(int left, int right)
{
    return (left + right)*0.125 / 2000;
}
double getPa(int left, int right)
{
    return (right - left)*0.125 / (1000*WHEEL_SEP);
}



int main(int argc, char** argv)
{
    double start_time = 0;
    bool flag = false;
    FILE * fd = fopen(BATT_LOG_FILE,"w");

    parse_args(argc, argv);

    //set signal handler to capture "ctrl+c" event
    if (signal(SIGINT, signalHandler) == SIG_ERR)
    {
        printf("signal(2) failed while setting up for SIGINT");
        return -1;
    }


    try
    {
        PlayerCc::PlayerClient client(gHostname, gPort);
        PlayerCc::Position2dProxy  pp(&client, gIndex);
        PlayerCc::IrProxy ir(&client, gIndex);
        PlayerCc::PowerProxy power(&client, gIndex);

        while (userQuit != 1)
        {
            client.Read();

            leftwheel = 300;
            rightwheel = 300;

            //set start time
            if (!flag)
            {
                flag = true;
                start_time = power.GetDataTime();
            }

            fprintf(fd,"%.01f %.03f \n", power.GetDataTime() - start_time, power.GetCharge());
	    
            printf("%.01f %.03f \n", power.GetDataTime() - start_time, power.GetCharge());
            for (int i = 0;i < 8;i++)
            {
                irdata[i] = (int)ir[i];
                leftwheel += weightleft[i] * (irdata[i] >> 4);
                rightwheel += weightright[i] * (irdata[i] >> 4);
            }
//            printf("%d %d %f %f\n", leftwheel, rightwheel, getPx(leftwheel, rightwheel), getPa(leftwheel, rightwheel));

            pp.SetSpeed( getPx(leftwheel, rightwheel), getPa(leftwheel, rightwheel));
        }

        usleep(500000);
        pp.SetSpeed(0, 0);
	fclose(fd);

    }
    catch (PlayerCc::PlayerError e)
    {
        std::cerr << e << std::endl;
        return -1;
    }
    return 1;
}


void signalHandler(int dummy)
{
    printf("Ctrl+C captured, exit program!\n");
    userQuit = 1;
}
