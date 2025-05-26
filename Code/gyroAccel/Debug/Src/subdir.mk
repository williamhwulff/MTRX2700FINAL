################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -D__UINT32_TYPE__ -D__INT32_TYPE__ -c -I"/Users/willrumi/STM32CubeIDE/final/F3-components" -I"/Users/willrumi/STM32CubeIDE/final/F3-components/Inc" -I"/Users/willrumi/STM32CubeIDE/final/F3-components/Src" -I"/Users/willrumi/Desktop/MTRX2700FINAL/Code/gyroAccel/Core/Inc" -I"/Users/willrumi/Desktop/MTRX2700FINAL/Code/gyroAccel/Drivers/STM32F3xx_HAL_Driver/Inc/Legacy" -I"/Users/willrumi/Desktop/MTRX2700FINAL/Code/gyroAccel/Drivers/CMSIS/Device/ST/STM32F3xx/Include" -I"/Users/willrumi/Desktop/MTRX2700FINAL/Code/gyroAccel/Drivers/CMSIS/Include" -I"/Users/willrumi/Desktop/MTRX2700FINAL/Code/gyroAccel/Drivers/STM32F3xx_HAL_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

