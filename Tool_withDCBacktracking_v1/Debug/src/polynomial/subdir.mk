################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/polynomial/Monom2.cpp \
../src/polynomial/MyList.cpp \
../src/polynomial/Polynom.cpp 

OBJS += \
./src/polynomial/Monom2.o \
./src/polynomial/MyList.o \
./src/polynomial/Polynom.o 

CPP_DEPS += \
./src/polynomial/Monom2.d \
./src/polynomial/MyList.d \
./src/polynomial/Polynom.d 


# Each subdirectory must supply rules for building sources it contributes
src/polynomial/%.o: ../src/polynomial/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O3 -c -fmessage-length=0 -no-pie -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


