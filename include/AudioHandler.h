/**
 * AudioHandler stores the audio environment data in the simulation.
 * It provides interfaces for storing audio information and for accessing audio information.
 * A number of simplifications about sound have been made in this class for the purpose of creating a simulation:<p>
 * <ul>
 * <li>Electrical watts and sound intensity watts are the same (it's all joules per second after all).</li>
 * <li>there is no interference between tones of different frequencies</li>
 * <li>All frequencies decay over distance at the same rate</li>
 * <li>The environment has no reverberations to confuse the direction and level of a sound</li>
 * <li>Robots don't move whilst making a sound</li>
 * <li>Obstacles in the environment allow sound to pass through them as if they (the obstacles) were not there</li>
 * </ul>
 * AudioHandler is accessed by the EPuck class.
 *
 * The AudioHandler code is not intended to be user facing, the user interacts with it using the EPuck API.
 *
 * AudioHandler uses the Singleton design pattern, so there can only ever be one instance of it. Constructor is called using the Instance() method.
 *
 * @date 26/07/2010
 * @author Jennifer Owen
 */

#ifndef AUDIOHANDLER_H_
#define AUDIOHANDLER_H_

#include <stdio.h>
#include <time.h>
#include <math.h>
#include "libplayerc++/playerc++.h"



/**
 * takes care of all audio processing and collection, stores audio data and
 * takes care of returning it to the calling object.
 * Audio in the simulation is global so AudioHandler is handled as a singleton
 * and there can be only one AudioHandler,
 * which contains all the audio data in the environment.
 **/
class AudioHandler
{
public:

	static const int sampleRate	=	8000;
	static const int fftBlockSize =	128;

	/**The quantisation lower boudaries for the frequency spectrum.
	 * The Fourier transform performed on the audio data quantises it into blocks.
	 * These values indicate the lower frequency range of each block.
	 * This is calculated by:<p>
	 * block number * sampleRate/fftBlockSize */
	double lowerFFTBounds[fftBlockSize/2];
	/**
	 * This data structure is used to send audio data from the AudioHandler to the EPuck API
	 * */
	typedef struct audio_message
	{
		/**the frequency of the tone heard*/
		double frequency;
		/**The direction, with respect to the robot's current heading, that the sound came from.
		 * In degrees, in front of the robot being 0 and anti-clockwise from there increasing.*/
		int direction;
		/**The distance from the listening robot that the tone originated.*/
		double distance;

	}audio_message_t;




	/** AudioBin stores information about a tone in the environment. Each object represents a frequency band containing tones being played.
	 * */
	class AudioBin
	{
	public:
		/**Represents a tone being played*/
		typedef struct audio_tone_t
		{
			/**The x coord of tone source*/
			double tx;
			/**The y coord of tone source*/
			double ty;
			/**The sound intensity level of the tone at source. Watts are used instead of volts (which is what the user gives) because this
			 * takes into account the impedance of the speaker and the microphones.*/
			//double wattsAtSource;
			/**Time that the tone will stop playing*/
			double end;
			/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
			audio_tone_t *next;
			/**Audio tones are stored as a linked list in an audio bin, previous and next are ways of navigating the linked list*/
			audio_tone_t *previous;
		}audio_tone_t;

		/**frequency bin lower bound*/
		double lowerFrequencyBound;
		/**Linked list of tones currently in this bin*/
		audio_tone_t *tones;

		/**The number of tones stored in this bin*/
		int numberTones;

		/**AudioBins are stored as a linked list by the handler so need to be able to navigate LL.*/
		AudioBin *next;
		/**AudioBins are stored as a linked list by the handler so need to be able to navigate LL.*/
		AudioBin *previous;

		AudioBin(double lowerFreq, AudioBin *prev, AudioBin *nxt);
		virtual ~AudioBin();

		int getNumberTones(void);
		int updateList(double currentTime);
		void addTone(double x, double y, double endtime);
		int calculateRawToneDataForPosition(double x, double y, double yaw, audio_message_t* output, int numMsgs);


	private:
		int removeTone(audio_tone_t *del);
		double getSoundIntensity(double levelAtSource, double distance);
		int convertDifferentialCoordsIntoBearing(double xdiff, double ydiff, double recieverYaw);

		//==== USEFUL STUFF ====

		int radiansToDegrees(double rads);
		double degreesToRadians(int degrees);
		double roundToNearest(double input, double resolution);
	};


	//function prototypes

	//singleton thing...
	static AudioHandler* GetAudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name);
	virtual ~AudioHandler();

	//methods.
	/**Returns the number of blocks in the fourier transform we run.*/
	int getFFTBlockSize(void);
	void playTone(int freq, double duration, char* name);
	int getNumberOfTones(void);
	int getTones(char* robotName, audio_message_t *store, size_t storesize);



	/** Test function to print all of the sound environment data to stdout.
	 * */
	void dumpData_TEST(void);

protected:
	//protected so it can be a singleton

	AudioHandler(PlayerCc::PlayerClient *simulationClient, PlayerCc::SimulationProxy *sim, char* name);

private:
	//member variables
	//linked list of AudioBins for the frequencies currently being played.
	AudioBin *environment;

	int numberOfBins;
	boost::thread updateAudioBinListThread;

	//player stuff
	PlayerCc::SimulationProxy	*simProxy;
	PlayerCc::PlayerClient *simClient;
	char aRobotName[32];

	//singleton reference to only instance of audiohandler.
	static AudioHandler* _instance;



	int removeBin(AudioBin *del);

	void updateAudioBinListThreaded(void);
};



#endif /* AUDIOHANDLER_H_ */
