/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "motor.h"
#include "servo.h"
#include "ftm_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define MOTOR_DIR_PORT                  (PTE)
#define MOTOR_DIR_PIN                   (14U)
#define IDLE_RPM                        (1900U)
#define MAX_SPEED                       (215U)
#define MIN_GEAR_FACTOR                 (0.2F)
#define ACCELERATION_SMOOTHING          (0.25F)
#define BRAKING_FACTOR                  (0.2F)
#define BRAKING_ADJUSTMENT              (0.6F)
#define SPEED_DELAY_MS                  (10U)
#define SPEED_MIN_THRESHOLD             (0.001F)

#define RPM_SMOOTHING_BASE              (0.8F)
#define RPM_SMOOTHING_LOW               (0.2F)
#define RPM_SMOOTHING_HIGH              (0.97F)
#define RPM_SMOOTHING_HIGH_GEAR         (0.03F)

#define RPM_SHIFT_UP_BASE               (0.6F)
#define RPM_SHIFT_STEP                  (0.07F)
#define RPM_SHIFT_MIN                   (0.2F)

#define RPM_SHIFT_DOWN_BASE             (1.4F)
#define RPM_SHIFT_DOWN_MAX              (2.0F)

#define NEUTRAL_GEAR                    (0U)
#define PWM_OFFSET                      (0U)

#define ACCELERATION_MAX_PERCENT        (100.0F)
#define GEAR_FACTOR_STEP                (0.1F)
#define GEAR_FACTOR_BASE                (1.0F)
#define BRAKING_DIVISOR                 (1.0F)
#define BRAKING_THRESHOLD_MULTIPLIER    (10U)
#define ADD_SPEED_LIMIT_GEAR            (2U)
#define ADD_SPEED_LIMIT_OFFSET          (1U)
#define RPM_SMOOTHING_WEIGHT            (1.0F)

/* Global variables --------------------------------------------------------------- */
/* current RPM (Revolutions Per Minute) */
static uint16_t ui16Rpm = IDLE_RPM;
/* duty cycle needed by the motor to display the current RPM (Revolutions Per Minute) */
uint16_t ui16MotorDutyCycle = 0U;

/* speed thresholds for each gear */
uint8_t ui8GearSpeed[MAX_GEAR + 1] = { 3U, 20U, 40U, 70U, 110U, 160U, 200U };
/* approximate gear ratios */
float fGearRatios[MAX_GEAR + 1] = { 0.0F, 10.0F, 7.0F, 5.0F, 3.0F, 2.0F, 1.5F };

/* current speed (Not showed anywhere. For now...) */
float fSpeed = 0.0F;
uint8_t ui8PreviousGear = 0;
uint8_t ui8LastGear = 0;

/* Functions ---------------------------------------------------------------------- */
/* @brief: Update speed based on acceleration and gear.
 *
 * @param ui8Acceleration: current acceleration percentage (0–100)
 * @param type: uint8_t
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_005
 */
void v_updateSpeed(uint8_t ui8Acceleration) {
    uint8_t ui8CurrentGear = ui8_getCurrentGear();

    /* convert acceleration (0–100) to a value between 0 and 1 */
    float acceleration_factor = (float)ui8Acceleration / ACCELERATION_MAX_PERCENT;

    /* adjust acceleration based on gear: lower gears accelerate faster (1.1, 1, 0.9, 0.8, 0.7, 0.6, 0.5) */
    float gearFactor = GEAR_FACTOR_BASE - (ui8CurrentGear - 1U) * GEAR_FACTOR_STEP;
    /* make sure gearFactor doesn’t go below 0.2 */
    if (gearFactor < MIN_GEAR_FACTOR)
        gearFactor = MIN_GEAR_FACTOR;

    /* combine acceleration and gear effect */
    acceleration_factor *= gearFactor;

    /* add a hysteresis to gear shift in order to prevent gear fluctuations */
    float shiftUpThreshold = (fGearRatios[ui8CurrentGear] + GEAR_CHANGE_THRESHOLD) * 1.5F;
    /* maximum possible speed should be over the shift threshold to be sure we do the gear change */
    float maxPossibleSpeed = ui8GearSpeed[ui8CurrentGear] + shiftUpThreshold;

    /* calculate braking effect: stronger braking at low acceleration */
    float braking = (BRAKING_DIVISOR / (ui8Acceleration + BRAKING_DIVISOR));
    if ((ui8Acceleration < (ui8CurrentGear * BRAKING_THRESHOLD_MULTIPLIER)) && (ui8PreviousGear > ui8CurrentGear)){
        braking += BRAKING_DIVISOR - gearFactor;
    }

    /* increase speed based on acceleration */
    float addSpeed = acceleration_factor * (maxPossibleSpeed - fSpeed) * ACCELERATION_SMOOTHING;
    if ((addSpeed > ui8CurrentGear)  && (ui8CurrentGear >= ADD_SPEED_LIMIT_GEAR)){
        addSpeed = ui8CurrentGear - ADD_SPEED_LIMIT_OFFSET;
    }

    fSpeed += addSpeed;

    /* decrease speed based on braking force */
    float decreaseSpeed = braking * (fSpeed * BRAKING_FACTOR);


    /*This logic helps simulate engine braking or resistance when the vehicle is climbing a hill:
    In high-speed reduction scenarios, the system limits the braking force to avoid excessive deceleration.
    In low gears (typically used for climbing), it increases braking force proportionally to the gear to simulate the extra resistance needed.*/

    /*if (decreaseSpeed > 6.0F){
        decreaseSpeed = ui8CurrentGear;
    }

    if ((decreaseSpeed > 1.0F) && (ui8CurrentGear <= 2U) && (ui8CurrentGear != 0)){
        decreaseSpeed = BRAKING_ADJUSTMENT * ui8CurrentGear;
    }*/

    fSpeed -= decreaseSpeed;

    /* add a small delay if speed is not zero */
    if (fSpeed != 0.0F) {
        OSIF_TimeDelay(SPEED_DELAY_MS);
    }

    /* make sure speed stays within valid range */
    if (fSpeed < SPEED_MIN_THRESHOLD)
        fSpeed = 0.0F;
    if (fSpeed > MAX_SPEED)
        fSpeed = MAX_SPEED;

    if (ui8CurrentGear != ui8LastGear) {
        if (ui8CurrentGear == MAX_GEAR)
            ui8PreviousGear = MAX_GEAR + 1U;
        else
            ui8PreviousGear = ui8LastGear;
        ui8LastGear = ui8CurrentGear;
    }
}

/* @brief: Retrieves the current simulated vehicle speed.
 *
 * @return: the current speed value
 * @return type: float
 */
float f_getSpeed(void){
    return fSpeed;
}

/* @brief: Calculate RPM (Revolutions Per Minute) based on speed, gear, and acceleration.
 *         RPM (Revolutions Per Minute) is affected by speed, gear ratio, acceleration, and a factor for slow RPM (Revolutions Per Minute) increase at high speed.
 *
 * @param ui8Acceleration: current acceleration percentage (0–100)
 * @param type: volatile uint8_t
 * @param ui8CurrentGear: current gear level
 * @param type: volatile uint8_t
 * @param ui16Rpm: global variable representing current engine RPM (Revolutions Per Minute)
 * @param type: uint16_t
 *
 * @return: no return value
 * @return type: void
 */
void v_calculateRPM(uint8_t ui8Acceleration){
    uint8_t ui8CurrentGear = ui8_getCurrentGear();
    float rpmBase;
    /* if gear is neutral (0), skip RPM (Revolutions Per Minute) calculation */
    if (ui8CurrentGear == NEUTRAL_GEAR){
    	ui16Rpm = 0;
        return;
    }

    /* calculate base RPM (Revolutions Per Minute) based on acceleration */
    rpmBase = IDLE_RPM + (MAX_RPM - IDLE_RPM) * (1.0F - exp(-ui8Acceleration / 20.0F));

    /* reduce RPM (Revolutions Per Minute) growth as gear increases */
    float gearFactor = RPM_SMOOTHING_WEIGHT / (RPM_SMOOTHING_WEIGHT + (ui8CurrentGear - 1) * GEAR_FACTOR_STEP);
    rpmBase *= gearFactor;

    /* smoothly update RPM (Revolutions Per Minute) using a weighted average */
    ui16Rpm = ui16Rpm * RPM_SMOOTHING_BASE + rpmBase * RPM_SMOOTHING_LOW;

    /* if in highest gear, allow RPM (Revolutions Per Minute) to reach max more gradually */
    if (ui8CurrentGear == MAX_GEAR) {
        rpmBase = MAX_RPM;
        ui16Rpm = ui16Rpm * RPM_SMOOTHING_HIGH + rpmBase * RPM_SMOOTHING_HIGH_GEAR;
    }

    /* keep RPM (Revolutions Per Minute) within allowed range */
    if ((ui16Rpm < IDLE_RPM) && (ui8CurrentGear != NEUTRAL_GEAR))
        ui16Rpm = IDLE_RPM;
    if (ui16Rpm > MAX_RPM)
        ui16Rpm = MAX_RPM;
}

/* @brief: Updates the engine RPM (Revolutions Per Minute) based on gear changes.
 *
 * @return: no return value
 * @return type: void
 */
void v_updateRPM(void){
    uint8_t ui8CurrentGear = ui8_getCurrentGear();
    int previousGear = ui8_getPreviousGear();

    /* if gear has changed, adjust RPM (Revolutions Per Minute) accordingly */
    if (previousGear != ui8CurrentGear) {
        float rpmChangeFactor;

        /* determine the RPM (Revolutions Per Minute) change factor based on the gear level */
        if (ui8CurrentGear > previousGear) {
            /* shifting up: reduce RPM (Revolutions Per Minute) more in lower gears, less in higher gears (0.57, 0.6, 0.53, 0.46, 0.39, 0.32, 0.25) */
            rpmChangeFactor = RPM_SHIFT_UP_BASE - (ui8CurrentGear - 1U) * RPM_SHIFT_STEP;
            if (rpmChangeFactor < RPM_SHIFT_MIN)
                rpmChangeFactor = RPM_SHIFT_MIN;
        } else {
            if (ui8CurrentGear == NEUTRAL_GEAR)
                rpmChangeFactor = 0.0F;
            else {
                /* shifting down: increase RPM (Revolutions Per Minute) more in lower gears, less in higher gears (1.33, 1.4, 1.47, 1.54, 1.61, 1.68, 1.75) */
                rpmChangeFactor = RPM_SHIFT_DOWN_BASE + (ui8CurrentGear - 1U) * RPM_SHIFT_STEP;
                if (rpmChangeFactor > RPM_SHIFT_DOWN_MAX)
                    rpmChangeFactor = RPM_SHIFT_DOWN_MAX;
            }
        }
        ui16Rpm = ui16Rpm * rpmChangeFactor;

        /* clamp RPM (Revolutions Per Minute) within valid range */
        if ((ui16Rpm < IDLE_RPM) && (ui8CurrentGear != NEUTRAL_GEAR))
			ui16Rpm = IDLE_RPM;
		if (ui16Rpm > MAX_RPM)
			ui16Rpm = MAX_RPM;
    }
}

/* @brief: Calculate the values to be sent to the motor to reflect the current RPM (Revolutions Per Minute).
 *         The motor has 2 pins for direction: In1 and In2.
 *         To set the motor forward we need to set In1 to HIGH and In2 to LOW.
 *         Since the pin In2 is not connected it is considered as LOW.
 *         We need to set In1 to HIGH in order to have a valid direction.
 *
 * @param ui16Rpm: current motor speed in revolutions per minute
 * @param type: uint16_t
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_014
 */
void v_displayRPM(ftm_instanceConfig_t* ftmMotorInstance){
    /* convert RPM (Revolutions Per Minute) to duty cycle (0 to 100%) */
    ui16MotorDutyCycle = ui16_scaleToDutyCycle(ui16Rpm, RPM_MAX_DUTY_CYCLE, 0U, MAX_RPM);

    /* send the calculated duty cycle to the motor controller */
    FTM_DRV_UpdatePwmChannel(ftmMotorInstance->FTM_INSTANCE,
                             ftmMotorInstance->hwChannelId,
                             FTM_PWM_UPDATE_IN_DUTY_CYCLE,
                             (uint16_t) ui16MotorDutyCycle,
                             PWM_OFFSET,
                             true);
}

/* @brief: Sets the motor direction and updates the PWM (Pulse Width Modulation) duty cycle.
 *
 * @return: no return value
 * @return type: void
 */
void v_setMotorDirection(ftm_instanceConfig_t* ftmMotorInstance){
    MOTOR_DIR_PORT->PSOR = (1U << MOTOR_DIR_PIN);

    FTM_DRV_UpdatePwmChannel(ftmMotorInstance->FTM_INSTANCE,
                             ftmMotorInstance->hwChannelId,
                             FTM_PWM_UPDATE_IN_DUTY_CYCLE,
                             (uint16_t) RPM_MAX_DUTY_CYCLE,
                             PWM_OFFSET,
                             true);
}

/* @brief: Retrieves the current RPM (Revolutions Per Minute) value.
 *
 * @return: the current RPM (Revolutions Per Minute) value.
 * @return type: uint16_t
 */
uint16_t ui16_getRpm(void){
    return ui16Rpm;
}

/* @brief: Provides access to the array containing speed values for each gear.
 *
 * @return: pointer to the gearSpeed array
 * @return type: uint8_t*
 */
uint8_t* ui8_getGearSpeedArray(void) {
    return ui8GearSpeed;
}

/* @brief: This test ensures the motor responds correctly across its full operating range.
 *         The function sends a PWM (Pulse Width Modulation) signal to the motor to run at maximum duty cycle,
 *         waits for a short delay, then sends another PWM (Pulse Width Modulation) signal to run at minimum duty cycle.
 *
 * @return: no return value
 * @return type: void
 */

void v_motorSelfTest(uint8_t ui8Gear, ftm_instanceConfig_t* ftmMotorInstance) {
    /* set motor to maximum power */
    uint16_t ui16Rpm;
    if (ui8Gear == NEUTRAL_GEAR)
        ui16Rpm = RPM_MAX_DUTY_CYCLE;
    else
        ui16Rpm = RPM_MIN_DUTY_CYCLE;
    FTM_DRV_UpdatePwmChannel(ftmMotorInstance->FTM_INSTANCE,
                             ftmMotorInstance->hwChannelId,
                             FTM_PWM_UPDATE_IN_DUTY_CYCLE,
                             (uint16_t) ui16Rpm,
                             PWM_OFFSET,
                             true);
}

/* @brief: Applies brakes to smoothly but quickly decelerate the motor. */
void v_applyBrakes(uint8_t ui8BrakeLevel, uint8_t * acceleration) {
	*acceleration = 0;
	return;
    float decayFactor = 1.0F - (ui8BrakeLevel * 0.05F);
    *acceleration *= decayFactor;
    if(*acceleration  < 15) *acceleration = 0;
}
