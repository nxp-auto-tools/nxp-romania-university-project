/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOTOR_H
#define MOTOR_H

/* Include files ------------------------------------------------------------------ */
#include "sdk_project_config.h"
#include <math.h>
#include "ftm_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define MAX_RPM               (7000U)
#define MOTOR_PWM_CHANNEL     (1U)
#define RPM_MAX_DUTY_CYCLE    (32768U)
#define RPM_MIN_DUTY_CYCLE    (9644U)

/* Function prototypes ------------------------------------------------------------ */
/* @brief: Update speed based on acceleration and gear. */
void v_updateSpeed(uint8_t ui8Acceleration) ;
/* @brief: Retrieves the current simulated vehicle speed. */
float f_getSpeed(void);
/* @brief: Calculate RPM (Revolutions Per Minute) based on speed, gear, and acceleration. */
void v_calculateRPM(uint8_t ui8Acceleration);
/* @brief: Updates the engine RPM (Revolutions Per Minute) based on gear changes. */
void v_updateRPM(void);
/* @brief: Calculate the values to be sent to the motor to reflect the current RPM (Revolutions Per Minute). */
void v_displayRPM(ftm_instanceConfig_t* ftmMotorInstance);
/* @brief: Sets the motor direction and updates the PWM (Pulse Width Modulation) duty cycle. */
void v_setMotorDirection(ftm_instanceConfig_t* ftmMotorInstance);
/* @brief: Retrieves the current RPM (Revolutions Per Minute) value. */
uint16_t ui16_getRpm(void);
/* @brief: Provides access to the array containing speed values for each gear. */
uint8_t* ui8_getGearSpeedArray(void);
/* @brief: This test ensures the motor responds correctly across its full operating range. */
void v_motorSelfTest(uint8_t ui8Gear, ftm_instanceConfig_t* ftmMotorInstance);
/* @brief: Applies brakes to smoothly but quickly decelerate the motor using proportional level (0-7). */
void v_applyBrakes(uint8_t ui8BrakeLevel, uint8_t * acceleration);

#endif /* MOTOR_H */
