################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/PolyDiVe.cpp \
../src/Verifizierer.cpp 

OBJS += \
./src/PolyDiVe.o \
./src/Verifizierer.o 

CPP_DEPS += \
./src/PolyDiVe.d \
./src/Verifizierer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


