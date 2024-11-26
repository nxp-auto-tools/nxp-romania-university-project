################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_driver.c \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_hw_access.c \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_irq.c 

OBJS += \
./SDK/platform/drivers/src/lpuart/lpuart_driver.o \
./SDK/platform/drivers/src/lpuart/lpuart_hw_access.o \
./SDK/platform/drivers/src/lpuart/lpuart_irq.o 

C_DEPS += \
./SDK/platform/drivers/src/lpuart/lpuart_driver.d \
./SDK/platform/drivers/src/lpuart/lpuart_hw_access.d \
./SDK/platform/drivers/src/lpuart/lpuart_irq.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/platform/drivers/src/lpuart/lpuart_driver.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_driver.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/lpuart/lpuart_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

SDK/platform/drivers/src/lpuart/lpuart_hw_access.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_hw_access.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/lpuart/lpuart_hw_access.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

SDK/platform/drivers/src/lpuart/lpuart_irq.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/platform/drivers/src/lpuart/lpuart_irq.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/platform/drivers/src/lpuart/lpuart_irq.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


