################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../interface/dialog_choose_mode.cpp \
../interface/dialog_delete.cpp \
../interface/dialog_error.cpp \
../interface/dialog_file_conflict.cpp \
../interface/dialog_info.cpp \
../interface/widget_ask_at_the_end.cpp \
../interface/widget_behavior.cpp \
../interface/widget_synchronization_log.cpp \
../interface/window_ask_at_the_end.cpp \
../interface/window_card.cpp \
../interface/window_config_main.cpp \
../interface/window_default_behavior.cpp \
../interface/window_log.cpp 

OBJS += \
./interface/dialog_choose_mode.o \
./interface/dialog_delete.o \
./interface/dialog_error.o \
./interface/dialog_file_conflict.o \
./interface/dialog_info.o \
./interface/widget_ask_at_the_end.o \
./interface/widget_behavior.o \
./interface/widget_synchronization_log.o \
./interface/window_ask_at_the_end.o \
./interface/window_card.o \
./interface/window_config_main.o \
./interface/window_default_behavior.o \
./interface/window_log.o 

CPP_DEPS += \
./interface/dialog_choose_mode.d \
./interface/dialog_delete.d \
./interface/dialog_error.d \
./interface/dialog_file_conflict.d \
./interface/dialog_info.d \
./interface/widget_ask_at_the_end.d \
./interface/widget_behavior.d \
./interface/widget_synchronization_log.d \
./interface/window_ask_at_the_end.d \
./interface/window_card.d \
./interface/window_config_main.d \
./interface/window_default_behavior.d \
./interface/window_log.d 


# Each subdirectory must supply rules for building sources it contributes
interface/%.o: ../interface/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	gcc `pkg-config --cflags --libs gio-2.0` `pkg-config --cflags --libs gtk+-2.0` -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


