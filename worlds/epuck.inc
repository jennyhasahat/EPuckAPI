
# The epuck bumper array
define epuck_ir ranger
(
  scount 8
  # define the pose of each transducer [xpos ypos heading]
  # number and location based on D1.1 deliverables
  spose[0] [0.030  -0.010   342.8]
  spose[1] [0.022  -0.025   314.2]	
  spose[2] [ 0.0    -0.031    270]
  spose[3] [ -0.03   -0.015   208.5]
  spose[4] [ -0.03  0.015   151.5]
  spose[5] [ 0.0   0.031   90]
  spose[6] [0.022 0.025   405.8]
  spose[7] [0.03  0.01    377.2]
  # define the field of view of each transducer [range_min range_max view_angle]
  # range is in meter (range based on D1.1 deliverables)
  sview [0.03 0.1 30]
  
  # define the size of each transducer [xsize ysize] in meters
  ssize [0.002 0.007]
  obstacle_return 0
  ranger_return 1
)

# epuck blobfinder
#define epuck_camera blobfinder
#(
#    channel_count 3
#    channels ["red" "green" "blue"]
#    range_max 0.5
#    obstacle_return 0
#    ranger_return 0
#)

# epuck wheels, just draw the wheels, no other purpose
define epuck_wheel model
(
  size [0.03 0.002]
  color "black"
  obstacle_return 0
  ranger_return 1
  blob_return 0 
)
define epuck_turret model
(
   size [0.061 0.046]
   polygons 1
   polygon[0].points 11
   polygon[0].point[0] [-0.035 0]
   polygon[0].point[1] [-0.028 -0.018 ]
   polygon[0].point[2] [0.011 -0.012 ]
   polygon[0].point[3] [0.026 -0.021 ]
   polygon[0].point[4] [0.027 -0.019 ]
   polygon[0].point[5] [0.015 -0.008 ]
   polygon[0].point[6] [0.015 0.008 ]
   polygon[0].point[7] [0.027 0.019 ]
   polygon[0].point[8] [0.026 0.021 ]
   polygon[0].point[9] [0.011 0.012 ]
   polygon[0].point[10] [-0.028 0.018 ]
   polygon[0].fill 0
)


# an epuck  with standard configuration
define epuck position
(
  # actual size
  size [0.07 0.07]

  # the epuck's center of rotation is offset from its center of area
  origin [0 0 0]

  # draw a nose on the robot so we can see which way it points
  gui_nose 1

  # estimated mass in KG
  mass 0.1 
  
  # this polygon approximates the shape of a epuck (R=3.5cm circle)
  polygons 1
  polygon[0].points 18
  polygon[0].point[0] [0 0.035]
  polygon[0].point[1] [0.0119707050160703 0.0328892417276262]
  polygon[0].point[2] [0.0224975663384939 0.0268115555096132]
  polygon[0].point[3] [0.0303108891319316 0.0175000000009072]
  polygon[0].point[4] [0.0344682713551847 0.00607768621971813]
  polygon[0].point[5] [0.0344682713557305 -0.00607768621662311]
  polygon[0].point[6] [0.0303108891335029 -0.0174999999981855]
  polygon[0].point[7] [0.0224975663409014 -0.026811555507593]
  polygon[0].point[8] [0.0119707050190235 -0.0328892417265513]
  polygon[0].point[9] [0 -0.035]
  polygon[0].point[10] [-0.011970705013117 -0.0328892417287011]
  polygon[0].point[11] [-0.0224975663360864 -0.0268115555116333]
  polygon[0].point[12] [-0.0303108891303602 -0.017500000003629]
  polygon[0].point[13] [-0.034468271354639 -0.00607768622281315]
  polygon[0].point[14] [-0.0344682713562762 0.0060776862135281]
  polygon[0].point[15] [-0.0303108891350743 0.0174999999954638]
  polygon[0].point[16] [-0.0224975663433089 0.0268115555055729]
  polygon[0].point[17] [-0.0119707050219767 0.0328892417254764]

  #left wheel
  epuck_wheel(origin [0 0.026 0])
  #right wheel
  epuck_wheel(origin [0 -0.026 0])
  epuck_turret()

  # differential steering model
  drive "diff"
  
  localization "gps"	  
  localization_origin [ -1.400 -1.400 0.000 ] 

  # ir sensor(sonar)
  epuck_ir()  

  ptz (
   # ptz properties   # ptz[0 0 180.0]   # ptz_speed[ 1.0 0.0 0.3]   
   size [0.005 0.01]
   watts 1.0  
   blobfinder 
   (
     channel_count 4
     channels ["red" "green" "blue" "orange"]
     range_max 0.5
     #ptz[0 0 180.0]
     obstacle_return 0
     pose [0.03 0.0 0.0]
   )
  )

  obstacle_return 1
  ranger_return 1
  blob_return 1
)


