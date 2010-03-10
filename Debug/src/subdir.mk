################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/EPuck.cc \
../src/TestEPuck.cc 

OBJS += \
./src/EPuck.o \
./src/TestEPuck.o 

CC_DEPS += \
./src/EPuck.d \
./src/TestEPuck.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/player-3.0/libplayerc++ -O0 -g3 -Wall -c -fmessage-length=0 `pkg-config --cflags playerc++` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


