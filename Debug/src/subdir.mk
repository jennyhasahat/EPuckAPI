################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/AudioBin.cc \
../src/AudioHandler.cc \
../src/EPuck.cc \
../src/TestEPuck.cc 

OBJS += \
./src/AudioBin.o \
./src/AudioHandler.o \
./src/EPuck.o \
./src/TestEPuck.o 

CC_DEPS += \
./src/AudioBin.d \
./src/AudioHandler.d \
./src/EPuck.d \
./src/TestEPuck.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/player-3.0/libplayerc++ -I"/home/jon/jenny/EPuckAPI/include" -O0 -g3 -Wall -c -fmessage-length=0 `pkg-config --cflags playerc++` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


