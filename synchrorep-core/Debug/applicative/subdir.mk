################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../applicative/application.cpp \
../applicative/execute.cpp \
../applicative/logging.cpp \
../applicative/new_synchronization.cpp 

OBJS += \
./applicative/application.o \
./applicative/execute.o \
./applicative/logging.o \
./applicative/new_synchronization.o 

CPP_DEPS += \
./applicative/application.d \
./applicative/execute.d \
./applicative/logging.d \
./applicative/new_synchronization.d 


# Each subdirectory must supply rules for building sources it contributes
applicative/%.o: ../applicative/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	gcc `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs gtk+-2.0` -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


