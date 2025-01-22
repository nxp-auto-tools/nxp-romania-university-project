################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/rtos/osif/osif_baremetal.c 

OBJS += \
./SDK/rtos/osif/osif_baremetal.o 

C_DEPS += \
./SDK/rtos/osif/osif_baremetal.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/rtos/osif/osif_baremetal.o: C:/nxp/S32DS.3.5/S32DS/software/S32SDK_S32K1XX_RTM_4.0.1/rtos/osif/osif_baremetal.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@SDK/rtos/osif/osif_baremetal.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


