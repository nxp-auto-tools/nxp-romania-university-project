/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SERVO_H
#define SERVO_H
/* Include files ------------------------------------------------------------------ */
#include "sdk_project_config.h"
#include "ftm_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define MAX_GEAR                 (6)
#define MIN_GEAR                 (1)
/* threshold for smooth gear changes */
#define GEAR_CHANGE_THRESHOLD    (8U)

/* Function prototypes ------------------------------------------------------------ */
/* @brief: Calculate the values to be sent to the servo motor in order to indicate the current gear. */
void v_displayGear(ftm_instanceConfig_t* ftmServoInstance);
/* @brief: This test ensures that the servo responds correctly across its full range of motion. */
void v_servoSelfTest(uint8_t ui8Gear, ftm_instanceConfig_t* ftmServoInstance);
/* @brief: Updates the current gear based on speed and acceleration, with smooth transitions. */
void v_updateGear(uint8_t ui8AccelerationZeroCounter);
/* @brief: Retrieves the current gear value. */
uint8_t ui8_getCurrentGear(void);
/* @brief: Retrieves the previous gear value. */
uint8_t ui8_getPreviousGear(void);

#endif /* SERVO_H */
