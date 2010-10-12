################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/testAPI.cc 

OBJS += \
./test/testAPI.o 

CC_DEPS += \
./test/testAPI.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/player-3.0/libplayerc++ -O0 -g3 -Wall -c -fmessage-length=0 `pkg-config --cflags playerc++` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


