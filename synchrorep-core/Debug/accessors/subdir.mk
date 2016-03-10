################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../accessors/ac_behavior.cpp \
../accessors/ac_common.cpp \
../accessors/ac_config.cpp \
../accessors/ac_logs.cpp \
../accessors/ac_preferences.cpp \
../accessors/db_access.cpp 

OBJS += \
./accessors/ac_behavior.o \
./accessors/ac_common.o \
./accessors/ac_config.o \
./accessors/ac_logs.o \
./accessors/ac_preferences.o \
./accessors/db_access.o 

CPP_DEPS += \
./accessors/ac_behavior.d \
./accessors/ac_common.d \
./accessors/ac_config.d \
./accessors/ac_logs.d \
./accessors/ac_preferences.d \
./accessors/db_access.d 


# Each subdirectory must supply rules for building sources it contributes
accessors/%.o: ../accessors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	gcc `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs gtk+-2.0` -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


