# epuck-mine.inc
# rough model of the epuck robots

#ranger sensors
define epuck_IR sensor
(
	# define the field of view of each transducer [range_min range_max view_angle]
	range [0.0064 0.1]
	
	# define the size of each transducer [xsize ysize] in meters
	size [0.007 0.002 0.004]
	
	#field of view
	fov 10
	
	#number of readings from range sensor
	#samples 1
	color_rgba [ 0 0 1 0.2 ] 
)

define epuck_IRs ranger
(
	# define the pose of each transducer [xpos ypos heading]
	epuck_IR ( pose [0.030	-0.010	-0.015	-15] )	#fr right		
	epuck_IR ( pose [0.022	-0.025	-0.015	-45] )	#fr right 2
	epuck_IR ( pose [0.0	-0.031	-0.015	-90] )	# right	
	epuck_IR ( pose [-0.03  -0.015	-0.015	-150] )	#back right
	epuck_IR ( pose [-0.03	0.015	-0.015	150] )	#back left
	epuck_IR ( pose [0.0  	0.031	-0.015	90] )	#left
	epuck_IR ( pose [0.022	0.025	-0.015	45] )	#fr left 2
	epuck_IR ( pose [0.03	0.01	-0.015	15] )	#fr left
	
		#angles got from the placement diagram on epuck website.
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
	epuck_IRs()  
	epuck_cam()
) 


