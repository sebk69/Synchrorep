################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../technical/tc_file_with_infos.cpp \
../technical/tc_files_binomial.cpp \
../technical/tc_find_files.cpp \
../technical/tc_misc.cpp \
../technical/tc_mount.cpp 

CC_SRCS += \
../technical/md5.cc 

OBJS += \
./technical/md5.o \
./technical/tc_file_with_infos.o \
./technical/tc_files_binomial.o \
./technical/tc_find_files.o \
./technical/tc_misc.o \
./technical/tc_mount.o 

CC_DEPS += \
./technical/md5.d 

CPP_DEPS += \
./technical/tc_file_with_infos.d \
./technical/tc_files_binomial.d \
./technical/tc_find_files.d \
./technical/tc_misc.d \
./technical/tc_mount.d 


# Each subdirectory must supply rules for building sources it contributes
technical/%.o: ../technical/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	gcc `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs gtk+-2.0` -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

technical/%.o: ../technical/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	gcc `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs gtk+-2.0` -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


