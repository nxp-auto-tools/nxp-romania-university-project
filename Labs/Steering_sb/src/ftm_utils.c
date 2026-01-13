/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "ftm_utils.h"

/* Functions ---------------------------------------------------------------------- */
/* @brief: Initializes a FTM (FlexTimer Module) instance for PWM (Pulse Width Modulation) control.
 *
 * @param ftmInstance: pointer to a structure containing configuration data for the FTM (FlexTimer Module) instance
 * @param type: ftm_instanceConfig*
 *
 * @return: no return value
 * @return type: void
 *
 */
void v_ftmInit(ftm_instanceConfig_t *ftmInstance) {
    /* initialize FTM (FlexTimer Module) instance - servo */
    FTM_DRV_Init(ftmInstance->instance,
                 ftmInstance->ftm_pwmInitConfig,
                 &(ftmInstance->ftm_stateStruct));
    /* initialize FTM PWM (FlexTimer Module Pulse Width Modulation) - servo
     * Implements: requirement SW_ST_003
     */
    FTM_DRV_InitPwm(ftmInstance->instance, ftmInstance->ftm_pwmConfig);
}

/* @brief: Scales an input value to a duty cycle between `ui16MinDutyCycle` and `ui16MaxDutyCycle`,
 *         then inverts the result to match the desired PWM (Pulse Width Modulation) behavior.
 *
 * @param inputValue: the input value to scale
 * @param type: uint16_t
 * @param ui16MaxDutyCycle: the maximum duty cycle value
 * @param type: uint16_t
 * @param ui16MinDutyCycle: the minimum duty cycle value
 * @param type: uint16_t
 * @param maxInputValue: the maximum possible input value
 * @param type: uint16_t
 *
 * @return: the scaled and inverted duty cycle value
 * @return type: uint16_t
 */
uint16_t ui16_scaleToDutyCycle(uint16_t inputValue,
                               uint16_t ui16MaxDutyCycle,
                               uint16_t ui16MinDutyCycle,
                               uint16_t maxInputValue) {
    uint16_t dutyCycle;
    /*

     * TODO: Implement the function to scale and invert the input value.

     *       - Don't forget to offset the result

     */

    return dutyCycle;
}
