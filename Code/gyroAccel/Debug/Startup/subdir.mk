################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Startup/startup_stm32f303vctx.s 

OBJS += \
./Startup/startup_stm32f303vctx.o 

S_DEPS += \
./Startup/startup_stm32f303vctx.d 


# Each subdirectory must supply rules for building sources it contributes
Startup/%.o: ../Startup/%.s Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -D__INT32_TYPE__ -DSTM32F303xC -D__UINT32_TYPE__ -c -I"/Users/willrumi/STM32CubeIDE/final/F3-components" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Startup

clean-Startup:
	-$(RM) ./Startup/startup_stm32f303vctx.d ./Startup/startup_stm32f303vctx.o

.PHONY: clean-Startup

