################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/others/format.cc 

CC_DEPS += \
./src/others/format.d 

OBJS += \
./src/others/format.o 


# Each subdirectory must supply rules for building sources it contributes
src/others/%.o: ../src/others/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


