/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FTM_CONTROL_H
#define FTM_CONTROL_H

/* Include files ------------------------------------------------------------------ */
#include "sdk_project_config.h"

/* Macro -------------------------------------------------------------------------- */
typedef struct {
    uint8_t FTM_INSTANCE;
    ftm_user_config_t *ftm_pwmInitConfig;
    ftm_pwm_param_t *ftm_pwmConfig;
    ftm_state_t ftm_stateStruct;
    uint8_t hwChannelId;
} ftm_instanceConfig_t;

/* Function prototypes ------------------------------------------------------------ */
/* @brief: Initializes a FTM (FlexTimer Module) instance for PWM (Pulse Width Modulation) control. */
void v_ftmInit(ftm_instanceConfig_t *ftmInstance);
/* @brief: Scales an input value to a duty cycle between `ui16MinDutyCycle` and `ui16MaxDutyCycle`,
 *         then inverts the result to match the desired PWM (Pulse Width Modulation) behavior.
 */
uint16_t ui16_scaleToDutyCycle(uint16_t inputValue,
                                uint16_t ui16MaxDutyCycle,
                                uint16_t ui16MinDutyCycle,
                                uint16_t maxInputValue);

#endif /* FTM_CONTROL_H */
