################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/simVectors/Random.cpp \
../src/simVectors/SimVector.cpp \
../src/simVectors/SimulationTable.cpp 

OBJS += \
./src/simVectors/Random.o \
./src/simVectors/SimVector.o \
./src/simVectors/SimulationTable.o 

CPP_DEPS += \
./src/simVectors/Random.d \
./src/simVectors/SimVector.d \
./src/simVectors/SimulationTable.d 


# Each subdirectory must supply rules for building sources it contributes
src/simVectors/%.o: ../src/simVectors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


