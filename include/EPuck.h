#ifndef EPUCK_H
#define EPUCK_H

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include "libplayerc++/playerc++.h"

#define THREADED 1

/**
 * A substitute for the player_blobfinder_blob_t structure. This one has all the variables in nice units (mostly ints),
 * instead of sucky ones (mostly uint32_t). Plus it doesn't require and Player headers to be included outside of this API.
 * Data is stored as member variables in the class, they are all public so can be accessed using the dot operator (.).
 * As in blobobject.variableName eg blobobject.x or blobobject.area
 * */
class Blob
{
	public:
		/**Index reference that the blobfinder proxy gave to this blob*/
		int	id;
		/**Colour of the blob in hex. Format is 0xaaRRGGBB, aa being the alpha component of the colour*/
		uint32_t colour;
		/**area of the blob*/
		int area;
		/**Centre of the blob, coordinates referenced by top left corner of camera image*/
		int x;
		/**Centre of the blob, coordinates referenced by top left corner of camera image*/
		int y;
		/**Left edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int left;
		/**right edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int right;
		/**top edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int top;
		/**bottom edge of bounding box for the blob [pixels] referenced by the top left corner of camera image.*/
		int bottom;
};


/**
Interacts with a simulated e-puck robot using Player commands.
This is a base class for the more specific SimulatedRobot and RealRobot classes, this class uses virtual functions so that polymorphism can be used.
<br>
Upon initialisation this class will create a thread using POSIX which reads in the sensor data from the robot. 
This allows multiple instances of EPuck to function in parrallel and read in their sensor data with no effort from the programmer.
*/
class EPuck
{
	public:
		//member variables
		int port; 
		char name[64];
		
		//player object member variables
		PlayerCc::PlayerClient		*epuck;

		PlayerCc::Position2dProxy	*p2dProxy;
		PlayerCc::SonarProxy		*sonarProxy;
		PlayerCc::BlobfinderProxy	*blobProxy;
		//robot also supports power, aio and blinkenlight proxies
		//as far as I can tell, stage does not support these

	
		EPuck(int robotPort, char* robotName);
		EPuck(char* robotName);
		~EPuck(void);
		
		void readSensors(void);
		
		// IR methods
		virtual double* getIRReadings(void);
		virtual double getIRReading(int index);
		virtual int getNumberOfIRs(void);

		// Blobfinder methods
		virtual int getCameraWidth(void);
		virtual int getCameraHeight(void);
		virtual int getNumberBlobs(void);
		virtual Blob getBlob(int index);

		// motor control methods
		virtual void setMotors(double forward, double turnrate);
		virtual void setDifferentialMotors(double left, double right);


	
	protected:
		double irReadings[8];

#if THREADED
	pthread_t readSensorsThread;
	void readSensorsThreaded(void);
	static void *startReadSensorThread(void *obj)
	{
		//All we do here is call the readSensorsThreaded() function
		reinterpret_cast<EPuck *>(obj)->readSensorsThreaded();
		return NULL;
	}

#endif
	
		

};




#endif
