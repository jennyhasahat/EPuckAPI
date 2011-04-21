# epuck-mine.inc
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
	#ssize [0.007 0.002]
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
		
		size [0.01 0.01 0.01]
)

define soundboard model
(
	obstacle_return 1
	ranger_return 1
	blob_return 1
	
	size [0.02 0.07 0.02]

	block
	(
		points 4
		point[0] [0 0]
		point[1] [0 0.07]
		point[2] [-0.01 0.07]
		point[3] [-0.01 0]
		z [0 0.01]
		color "green"
	)
	block
	(
		points 4
		point[0] [-0.005 0.01]
		point[1] [0.005 0.01]
		point[2] [0.005 0.02]
		point[3] [-0.005 0.02]
		z [0.005 0.015]
		color "black"
	)
	block
	(
		points 4
		point[0] [-0.005 0.03]
		point[1] [0.005 0.03]
		point[2] [0.005 0.04]
		point[3] [-0.005 0.04]
		z [0.005 0.015]
		color "black"
	)
	block
	(
		points 4
		point[0] [-0.005 0.05]
		point[1] [0.005 0.05]
		point[2] [0.005 0.06]
		point[3] [-0.005 0.06]
		z [0.005 0.015]
		color "black"
	)
	
)

# epuck base
define epuck position
(
	obstacle_return 1
	ranger_return 1
	blob_return 1
	gui_outline 1
	gravity_return 1
    
	# actual size
	size [0.07 0.07 0.045]
	color "DarkGreen"
	
	# estimated mass in KG
	mass 0.3
	
	#Defining the shape of an epuck
	block
	(
		points 8
		point[0] [1 0]
		point[1] [0.7071 0.7071]
		point[2] [0 1]
		point[3] [-0.7071 0.7071]
		point[4] [-1 0]
		point[5] [-0.7071 -0.7071]
		point[6] [0 -1]
		point[7] [0.7071 -0.7071]
		z [0 1]
	)
	
	soundboard(pose [0.03 0 0 0])

	# differential steering model
	drive "diff"

	# sensors attached to epuck
	epuck_IR()  
	epuck_cam()
) 


