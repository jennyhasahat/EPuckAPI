#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

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
	//TODO fix colours so that it works with the stuff you wrote for stage.
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
 * Stores information about the tones in a single frequency band that can be heard by the robot.
 * This is the information that the robot will be able to detect and is all robot-centric.
 * @see EPuck#getTone
 * @see EPuck#listenForTones
 * @see EPuck#getBlob
 * */
class Tone
{
public:
	/**The lower bound on the frequency range this tone could be*/
	double frequency;
	/**The volume of the tone. This is some arbitrary number without a real measurement, but they are consistent with each other so can be
	 * compared to other tones and volumes*/
	double volume;
	/**The bearing of the sound source with respect to the EPuck. If it is directly in front of the EPuck this will be 0,
	 * bearings are then measured in DEGREES anticlockwise from the robot's front.*/
	int bearing;
};



#endif
