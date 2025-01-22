################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_common.c \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_hw_access.c \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_pwm_driver.c 

OBJS += \
./SDK/platform/drivers/src/ftm/ftm_common.o \
./SDK/platform/drivers/src/ftm/ftm_hw_access.o \
./SDK/platform/drivers/src/ftm/ftm_pwm_driver.o 

C_DEPS += \
./SDK/platform/drivers/src/ftm/ftm_common.d \
./SDK/platform/drivers/src/ftm/ftm_hw_access.d \
./SDK/platform/drivers/src/ftm/ftm_pwm_driver.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/drivers/src/ftm/ftm_common.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_common.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/ftm/ftm_common.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

SDK/platform/drivers/src/ftm/ftm_hw_access.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_hw_access.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/ftm/ftm_hw_access.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

SDK/platform/drivers/src/ftm/ftm_pwm_driver.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/ftm/ftm_pwm_driver.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/ftm/ftm_pwm_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


