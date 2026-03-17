/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "motor.h"
#include "servo.h"
#include "ftm_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define MIN_DUTY_CYCLE                (320U)
#define MAX_DUTY_CYCLE                (1800U)
/* conversion factor from duty cycle to voltage */
#define DUTY_CYCLE_TO_VOLTS_SCALE     (1.25F)
/* if acceleration is read as 0 X times then turn off motor */
#define ACC_ZERO_COUNTER_THRESHOLD    (10U)
#define SERVO_SELFTEST_DELAY_MS       (200U)

#define NEUTRAL_GEAR                  (0U)
#define PWM_OFFSET                    (0U)

/* Global variables --------------------------------------------------------------- */
/* duty cycle needed by the servo to display the current gear */
static uint16_t ui16ServoDutyCycle = 0U;
static uint8_t ui8PreviousGear;
/* current gear (displayed by the servo position) */
static uint8_t ui8CurrentGear = 0U;

/* Functions ---------------------------------------------------------------------- */
/* @brief: Calculate the values to be sent to the servo motor in order to indicate the current gear.
 * EX: for 6 gears display:
 *                                 3rd gear
 *                          2nd gear  ^  4th gear
 *                      1st gear      |      5th gear
 * no acceleration (motor off)        |        6th gear
 *
 * HINT: at motor off servoDutyCycle is MIN_DUTY_CYCLE
 *
 * @param ui8CurrentGear: current gear level
 * @param type: volatile uint8_t
 * @param ui16ServoDutyCycle: global variable representing the PWM (Pulse Width Modulation) duty cycle for the servo
 * @param type: volatile uint16_t
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_015
 */
void v_displayGear(ftm_instanceConfig_t* ftmServoInstance) {
    /* calculate servo duty cycle based on current gear */
    ui16ServoDutyCycle = ui16_scaleToDutyCycle(ui8CurrentGear, MAX_DUTY_CYCLE, MIN_DUTY_CYCLE, MAX_GEAR);

    /* if gear is neutral (0), set duty cycle to maximum */
    if (ui8CurrentGear == NEUTRAL_GEAR)
        ui16ServoDutyCycle = MAX_DUTY_CYCLE;

    /* send the duty cycle value to the servo motor */
    FTM_DRV_UpdatePwmChannel(ftmServoInstance->FTM_INSTANCE,
            ftmServoInstance->hwChannelId,
            FTM_PWM_UPDATE_IN_TICKS,
            (uint16_t) ui16ServoDutyCycle,
            PWM_OFFSET,
            true);
}

/* @brief: This test ensures that the servo responds correctly across its full range of motion.
 *         The function sends a PWM (Pulse Width Modulation) signal to move the servo to the minimum duty cycle position,
 *         waits for a short delay, then sends another PWM (Pulse Width Modulation) signal to move it to the maximum duty cycle position.
 *
 * @return: no return value
 * @return type: void
 */
void v_servoSelfTest(uint8_t ui8Gear, ftm_instanceConfig_t* ftmServoInstance) {
    uint16_t ui16DutyCycle;
        ui16DutyCycle = ui16_scaleToDutyCycle(ui8Gear, MAX_DUTY_CYCLE, MIN_DUTY_CYCLE, MAX_GEAR);

        if (ui8Gear == NEUTRAL_GEAR)
            ui16DutyCycle = MAX_DUTY_CYCLE;

        FTM_DRV_UpdatePwmChannel(ftmServoInstance->FTM_INSTANCE,
                                 ftmServoInstance->hwChannelId,
                                 FTM_PWM_UPDATE_IN_TICKS,
                                 (uint16_t) ui16DutyCycle,
                                 PWM_OFFSET,
                                 true);
        OSIF_TimeDelay(SERVO_SELFTEST_DELAY_MS);
}

/* @brief: Updates the current gear based on speed and acceleration, with smooth transitions.
 *
 * @param fSpeed: current vehicle speed
 * @param type: float
 * @param ui8AccelerationZeroCounter: counter indicating how long acceleration has been zero
 * @param type: uint8_t
 * @param ui8CurrentGear: global or external variable representing the current gear
 * @param type: volatile uint8_t
 * @param ui16Rpm: global or external variable representing the current RPM (Revolutions Per Minute)
 * @param type: uint16_t
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_008, SW_TR_009, SW_TR_010, SW_TR_011, SW_TR_012
 */
void v_updateGear(uint8_t ui8AccelerationZeroCounter) {
    ui8PreviousGear = ui8CurrentGear;
    float fSpeed = f_getSpeed();

    uint8_t *ui8GearSpeed = ui8_getGearSpeedArray();// {3, 20, 40, 70, 110, 160, 200}

//  TODO 3.1: Shift from NEUTRAL (0) to GEAR 1
//  Conditions: speed > 0 AND gear == 0 AND acceleration active (ui8AccelerationZeroCounter == 0)
    if (-1/* COMPLETE HERE */) {
        ui8CurrentGear++;
    }
//  TODO 3.2: Shift from GEAR 1 to NEUTRAL
//  Conditions: speed <= threshold AND gear == 1 AND no acceleration for a while (counter > 10)
//  Hint: threshold = ui8GearSpeed[ui8CurrentGear] - GEAR_CHANGE_THRESHOLD
    else if (-1/* COMPLETE HERE */){
       ui8CurrentGear--;
    }
//  TODO 3.3: Shift UP (2→3, 3→4, etc.)
//  Conditions: speed > current threshold AND gear < max_gear AND gear != neutral_gear
//  Hint:
    else if (-1/* COMPLETE HERE */) {
        ui8CurrentGear++;
    }
//  TODO 3.4: Shift DOWN (6→5, 5→4, etc.)
//  Conditions: speed <= previous gear threshold AND gear > 1
    else if (-1/* COMPLETE HERE */) {
        ui8CurrentGear--;
    }
}

/* @brief: Retrieves the current gear value.
 *
 * @return: the current gear value
 * @return type: uint8_t
 */
uint8_t ui8_getCurrentGear(void){
    return ui8CurrentGear;
}

/* @brief: Retrieves the previous gear value.
 *
 * @return: the current gear value
 * @return type: uint8_t
 */
uint8_t ui8_getPreviousGear(void){
    return ui8PreviousGear;
}
