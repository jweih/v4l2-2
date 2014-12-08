################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/Application.cpp \
../Src/H264Encode.cpp \
../Src/H264OnDemandServerMediaSubsession.cpp \
../Src/V4L2.cpp \
../Src/V4L2FramedSource.cpp 

OBJS += \
./Src/Application.o \
./Src/H264Encode.o \
./Src/H264OnDemandServerMediaSubsession.o \
./Src/V4L2.o \
./Src/V4L2FramedSource.o 

CPP_DEPS += \
./Src/Application.d \
./Src/H264Encode.d \
./Src/H264OnDemandServerMediaSubsession.d \
./Src/V4L2.d \
./Src/V4L2FramedSource.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/jiqiujia/projects/v4l2/RTSPServer/Head" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


