/*
 * Copyright 2020, 2024-2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
/* core SDK and system utilities */
#include "sdk_project_config.h"
#include "osif.h"
#include <stdio.h>
#include <string.h>

/* application modules */
#include "motor.h"
#include "servo.h"
#include "adc_utils.h"
#include "ftm_utils.h"
#include "pdb_utils.h"
#include "clock_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define MAX_ACC                  (100U)
#define LOOP_UPDATE_DELAY_MS     (10U)
#define LOOP_CONTROL_DELAY_MS    (50U)
#define ADC_ZERO_THRESHOLD       (1U)
#define INITIAL_GEAR             (0)
#define ADC_CHANNEL_INDEX        (0U)

/* Global variables --------------------------------------------------------------- */
int iExitCode = 0;
static ftm_instanceConfig_t ftmServoInstance;
static ftm_instanceConfig_t ftmMotorInstance;

static adc_instanceConfig_t adcInstance;
static pdb_instanceConfig_t pdbInstance;

/* Function prototypes ------------------------------------------------------------ */
/* @brief: Performs a full self-test sequence for both the servo and motor components. */
void v_selfTest(ftm_instanceConfig_t* ftmMotorInstance, ftm_instanceConfig_t* ftmServoInstance);
/* @brief: Initializes all core system components required for application startup. */
void v_init(void);
/* @brief: Read the conversion result, store it into a variable and set a specified flag. */
void v_ADC_IRQHandler(void);
/* @brief: Installs and enables an interrupt handler. */
void v_interruptInit(IRQn_Type irqNumber, const isr_t isrHandler);

/* Main function ------------------------------------------------------------------ */
int main(void) {
    /* local variables */
    /* current acceleration level 0 - 100% */
    uint8_t ui8Acceleration = 0U;
    /* counter for consecutive zero-acceleration readings */
    uint8_t ui8AccelerationZeroCounter = 0U;

    v_init();
    v_selfTest(&ftmMotorInstance, &ftmServoInstance);

    /* infinite loop */
    while (1) {
        v_updateSpeed(ui8Acceleration);
        v_updateGear(ui8AccelerationZeroCounter);
        v_updateRPM();
        v_displayGear(&ftmServoInstance);
        v_calculateRPM(ui8Acceleration);
        v_displayRPM(&ftmMotorInstance);

        /* small delay between updates */
        OSIF_TimeDelay(LOOP_UPDATE_DELAY_MS);

        if (adcInstance.adcConvDone == true) {
            /* convert raw ADC (Analog to Digital Converter) value to acceleration (0–100%) */
            v_scaleAdcValue(MAX_ACC, &adcInstance.adcValue);
            ui8Acceleration = adcInstance.adcValue;

            /* track how long acceleration stays at 0 */
            if (ui8Acceleration < ADC_ZERO_THRESHOLD)
                ui8AccelerationZeroCounter++;
            else
                ui8AccelerationZeroCounter = 0U;

            /* reset the conversion flag */
            adcInstance.adcConvDone = false;
            /* start the next ADC (Analog to Digital Converter) conversion using the PDB timer */
            PDB_DRV_SoftTriggerCmd(pdbInstance.instance);
        }

        /* delay to control loop speed */
        OSIF_TimeDelay(LOOP_CONTROL_DELAY_MS);
    }

    return iExitCode;
}

/* Functions ---------------------------------------------------------------------- */
/* @brief: Performs a full self-test sequence for both the servo and motor components.
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_023
 */
void v_selfTest(ftm_instanceConfig_t* ftmMotorInstance, ftm_instanceConfig_t* ftmServoInstance) {
    uint8_t ui8Gear = INITIAL_GEAR;
    bool ascending = true;

    while (true) {
        v_motorSelfTest(ui8Gear, ftmMotorInstance);
        v_servoSelfTest(ui8Gear, ftmServoInstance);

        if (ui8Gear == MAX_GEAR) {
            ascending = false;
        } else if ((ui8Gear == 0) && (!ascending)) {
            break;
        }
        ui8Gear += ascending ? 1 : -1;
    }
}

/* @brief: Initializes all core system components required for application startup.
 *
 * @return: no return value
 * @return type: void
 * Implements: SW_TR_003
 */
void v_init(void) {
    adcInstance.instance = INST_ADC_1;
    adcInstance.chanIndex = ADC_CHANNEL_INDEX;
    adcInstance.channelConfig = &adc_1_ChnConfig0;
    adcInstance.converterConfig = &adc_1_ConvConfig0;
    adcInstance.adcConvDone = false;
    adcInstance.adcValue = 0U;
    adcInstance.irqNumber = ADC1_IRQn;
    adcInstance.isrHandler = &v_ADC_IRQHandler;

    ftmServoInstance.FTM_INSTANCE = INST_FLEXTIMER_PWM_SERVO;
    ftmServoInstance.ftm_pwmInitConfig = &flexTimer_pwm_servo_InitConfig;
    ftmServoInstance.ftm_pwmConfig = &flexTimer_pwm_servo_PwmConfig;
    ftmServoInstance.ftm_stateStruct = (ftm_state_t ){ 0 };
    ftmServoInstance.hwChannelId = flexTimer_pwm_servo_IndependentChannelsConfig[0].hwChannelId;

    ftmMotorInstance.FTM_INSTANCE = INST_FLEXTIMER_PWM_MOTOR;
    ftmMotorInstance.ftm_pwmInitConfig = &flexTimer_pwm_motor_InitConfig;
    ftmMotorInstance.ftm_pwmConfig = &flexTimer_pwm_motor_PwmConfig;
    ftmMotorInstance.ftm_stateStruct = (ftm_state_t ){ 0 };
    ftmMotorInstance.hwChannelId = flexTimer_pwm_motor_IndependentChannelsConfig[MOTOR_PWM_CHANNEL].hwChannelId;

    pdbInstance.instance = INST_PDB_1;
    pdbInstance.timerConfig = &pdb_1_timerConfig0;
    pdbInstance.pdlyTimeout_microS = PDLY_TIMEOUT;
    pdbInstance.chn = 0U;
    pdbInstance.preChn = 0U;
    pdbInstance.adcTrigConfig = &pdb_1_adcTrigConfig0;

    /* set up the system clock */
    v_clockInit();
    /* set up all the configured pins */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    v_adcInit(&adcInstance);

    INT_SYS_InstallHandler(adcInstance.irqNumber, adcInstance.isrHandler, (isr_t*)0);

    v_ftmInit(&ftmServoInstance);
    v_ftmInit(&ftmMotorInstance);

    v_pdbInit(&pdbInstance);
    v_setMotorDirection(&ftmMotorInstance);

    /* enable ADC (Analog to Digital Converter) interrupt */
    INT_SYS_EnableIRQ(adcInstance.irqNumber);
}

/* @brief: ADC (Analog to Digital Converter) Interrupt Service Routine.
 *         Read the conversion result, store it into a variable and set a specified flag.
 *
 * @return: no return value
 * @return type: void
 */
void v_ADC_IRQHandler(void) {
    ADC_DRV_GetChanResult(adcInstance.instance,
                          adcInstance.chanIndex,
                          (uint16_t*) &adcInstance.adcValue);

    adcInstance.adcConvDone = true;
}

/* @brief: Installs and enables an interrupt handler.
 *
 * @param irqNumber: the IRQ number representing the specific interrupt to initialize
 * @param type: IRQn_Type
 * @param isrHandler: the function pointer to the interrupt service routine to be installed
 * @param type: isr_t
 *
 * @return: no return value
 * @return type: void
 */
void v_interruptInit(IRQn_Type irqNumber, const isr_t isrHandler) {
    INT_SYS_InstallHandler(irqNumber, isrHandler, (isr_t*) 0);

    /* enable ADC (Analog to Digital Converter) 1 interrupt */
    INT_SYS_EnableIRQ(irqNumber);
}
