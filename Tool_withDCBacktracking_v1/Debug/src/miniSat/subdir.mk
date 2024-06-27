################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/miniSat/Options.cpp \
../src/miniSat/Solver.cpp 

OBJS += \
./src/miniSat/Options.o \
./src/miniSat/Solver.o 

CPP_DEPS += \
./src/miniSat/Options.d \
./src/miniSat/Solver.d 


# Each subdirectory must supply rules for building sources it contributes
src/miniSat/%.o: ../src/miniSat/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


