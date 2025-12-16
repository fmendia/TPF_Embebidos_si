################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/FTM.c \
../source/SysTick.c \
../source/WS2812.c \
../source/card.c \
../source/database.c \
../source/display_matrix.c \
../source/display_rtos.c \
../source/encoder.c \
../source/gpio.c \
../source/leds.c \
../source/main.c \
../source/menu.c \
../source/msg_pool.c \
../source/ui.c \
../source/user_db.c 

C_DEPS += \
./source/FTM.d \
./source/SysTick.d \
./source/WS2812.d \
./source/card.d \
./source/database.d \
./source/display_matrix.d \
./source/display_rtos.d \
./source/encoder.d \
./source/gpio.d \
./source/leds.d \
./source/main.d \
./source/menu.d \
./source/msg_pool.d \
./source/ui.d \
./source/user_db.d 

OBJS += \
./source/FTM.o \
./source/SysTick.o \
./source/WS2812.o \
./source/card.o \
./source/database.o \
./source/display_matrix.o \
./source/display_rtos.o \
./source/encoder.o \
./source/gpio.o \
./source/leds.o \
./source/main.o \
./source/menu.o \
./source/msg_pool.o \
./source/ui.o \
./source/user_db.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\ucosiii_config" -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\rtos\uCOSIII\src\uC-CPU" -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\rtos\uCOSIII\src\uC-LIB" -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"C:\Users\pipem\ITBA\2025\Embebidos\rtos_diap\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/FTM.d ./source/FTM.o ./source/SysTick.d ./source/SysTick.o ./source/WS2812.d ./source/WS2812.o ./source/card.d ./source/card.o ./source/database.d ./source/database.o ./source/display_matrix.d ./source/display_matrix.o ./source/display_rtos.d ./source/display_rtos.o ./source/encoder.d ./source/encoder.o ./source/gpio.d ./source/gpio.o ./source/leds.d ./source/leds.o ./source/main.d ./source/main.o ./source/menu.d ./source/menu.o ./source/msg_pool.d ./source/msg_pool.o ./source/ui.d ./source/ui.o ./source/user_db.d ./source/user_db.o

.PHONY: clean-source

