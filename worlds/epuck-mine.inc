# epuck.inc
# rough model of the epuck robots

#ranger sensors
define epuck_IR ranger
(
	# number of sonars
	scount 8

	# define the pose of each transducer [xpos ypos heading]
	spose[0] [0.030	-0.010	-15]	#fr right
	spose[1] [0.022	-0.025	-45]	#fr right 2
	spose[2] [0.0	-0.031	-90]	# right	
	spose[3] [-0.03   -0.015	-150]	#back right
	spose[4] [-0.03	0.015		150]	#back left
	spose[5] [0.0  	0.031		90]	#left
	spose[6] [0.022	0.025		45]	#fr left 2
	spose[7] [0.03	0.01		15]	#fr left
	
		#angles got from the placement diagram on epuck website.

		
	# define the field of view of each transducer [range_min range_max view_angle]
	sview [0.0064 0.1 10]
		# range min and max are from the epuck website prox sensor graphs.

	# define the size of each transducer [xsize ysize] in meters
	ssize [0.002 0.002]
		#from IR data sheet
)

# epuck blobfinder
define epuck_cam blobfinder
(
		# number of colours to look for
		colors_count 1
	
		# which colours to look for
		colors ["red"]
	
		# camera parameters
		image [640 480]		#resolution
		
		size [0.001 0.001 0.001]
)



# epuck base
define epuck position
(
	# actual size
	size [0.07 0.07 0.12]
	
	# estimated mass in KG
	mass 0.5
	
	#Defining the shape of an epuck
	bitmap "bitmaps/circle.png"
	gui_outline 1
	show_blocks 0

	# differential steering model
	drive "diff"

	# sensors attached to epuck
	epuck_IR()  
	epuck_cam()
) 


