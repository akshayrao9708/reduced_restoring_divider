################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/circuit/Circuit.cpp \
../src/circuit/Edge.cpp \
../src/circuit/EdgePointer.cpp \
../src/circuit/Node.cpp \
../src/circuit/NodePointer.cpp 

OBJS += \
./src/circuit/Circuit.o \
./src/circuit/Edge.o \
./src/circuit/EdgePointer.o \
./src/circuit/Node.o \
./src/circuit/NodePointer.o 

CPP_DEPS += \
./src/circuit/Circuit.d \
./src/circuit/Edge.d \
./src/circuit/EdgePointer.d \
./src/circuit/Node.d \
./src/circuit/NodePointer.d 


# Each subdirectory must supply rules for building sources it contributes
src/circuit/%.o: ../src/circuit/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


