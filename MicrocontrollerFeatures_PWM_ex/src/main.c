/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/*!
** @file main.c
** @version 01.00
** @brief
**         Main module.
**         This module contains user's application code.
*/
/* MODULE main */

/* Including needed modules to compile this module/procedure */
#include "sdk_project_config.h"
#include "osif.h"
#include <stdio.h>

volatile int exit_code = 0;

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
    /* Write your local variable definition here */
    ftm_state_t ftmStateStruct;
    int dutyCycle = 0U;

    /* Initialize clock module */
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    /* Initialize pins */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    /* Initialize FTM instance */
    FTM_DRV_Init(INST_FLEXTIMER_PWM_1, &flexTimer_pwm_1_InitConfig, &ftmStateStruct);

    /* Initialize FTM PWM */
    FTM_DRV_InitPwm(INST_FLEXTIMER_PWM_1, &flexTimer_pwm_1_PwmConfig);

    FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_1,
                             flexTimer_pwm_1_IndependentChannelsConfig[0].hwChannelId,
                             FTM_PWM_UPDATE_IN_TICKS, (uint16_t)0,
                             0U,
                             true);

    /* Infinite loop */
    while(1)
    {
        /* Increase the brightness */
        for (dutyCycle = 0; dutyCycle < 32768; dutyCycle += 25)
        {
            FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_1,
                                     flexTimer_pwm_1_IndependentChannelsConfig[0].hwChannelId,
                                     FTM_PWM_UPDATE_IN_TICKS, (uint16_t)dutyCycle,
                                     0U,
                                     true);
            OSIF_TimeDelay(1);
        }
        OSIF_TimeDelay(100);

        /* Decrease the brightness */
        for (dutyCycle = 32768; dutyCycle > 0; dutyCycle -= 25)
        {
            FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_1,
                                     flexTimer_pwm_1_IndependentChannelsConfig[0].hwChannelId,
                                     FTM_PWM_UPDATE_IN_TICKS, (uint16_t)dutyCycle,
                                     0U,
                                     true);
            OSIF_TimeDelay(1);
        }
        OSIF_TimeDelay(100);
    }

    return exit_code;
}
/* END main */
