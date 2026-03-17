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
#include "can_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define TX_MAILBOX 9
#define RX_MAILBOX 7

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

/* Buffer used to store received CAN messages */
flexcan_msgbuff_t rx_msg;
uint8_t ui8BrakeLevel = 0;

static flexcan_data_info_t rxInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U
};

static flexcan_data_info_t txInfo = { .data_length = 4U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U

};

/* Function prototypes ------------------------------------------------------------ */
/* @brief: Performs a full self-test sequence for both the servo and motor components. */
void v_selfTest(ftm_instanceConfig_t *ftmMotorInstance,
                ftm_instanceConfig_t *ftmServoInstance);
/* @brief: Initializes all core system components required for application startup. */
void v_init(void);
/* @brief: Read the conversion result, store it into a variable and set a specified flag. */
void v_ADC_IRQHandler(void);
/* @brief: Installs and enables an interrupt handler. */
void v_interruptInit(IRQn_Type irqNumber, const isr_t isrHandler);
/* @brief: Callback for FlexCAN to parse incoming messages. */
void flexcanCallback(uint8_t instance, flexcan_event_type_t eventType,
                     uint32_t buffIdx, flexcan_state_t *flexcanState);

/* Main function ------------------------------------------------------------------ */
int main(void) {
    uint8_t txData[4] = { 0 };
    uint8_t previousTxData[4] = { 0 };

    /* local variables */
    /* current acceleration level 0 - 100% */
    uint8_t ui8Acceleration = 0U;
    /* counter for consecutive zero-acceleration readings */
    uint8_t ui8AccelerationZeroCounter = 0U;
    v_init();
    v_selfTest(&ftmMotorInstance, &ftmServoInstance);

    /* infinite loop */
    while (1) {
    	// TODO 6.2: Add brake application here
    	// Hint: Check if ui8BrakeLevel > 0, then call v_applyBrakes()
    	/* COMPLETE HERE */
        v_updateSpeed(ui8Acceleration);
        v_updateGear(ui8AccelerationZeroCounter);
        v_updateRPM();
        v_displayGear(&ftmServoInstance);
        v_calculateRPM(ui8Acceleration);
        v_displayRPM(&ftmMotorInstance);

        /* small delay between updates */
        OSIF_TimeDelay(LOOP_UPDATE_DELAY_MS);


		if (ui8Acceleration < ADC_ZERO_THRESHOLD)
			ui8AccelerationZeroCounter++;
		else
			ui8AccelerationZeroCounter = 0U;

        if (adcInstance.adcConvDone == true && !ui8BrakeLevel) {
            /* convert raw ADC (Analog to Digital Converter) value to acceleration (0–100%) */
            v_scaleAdcValue(MAX_ACC, &adcInstance.adcValue);
            ui8Acceleration = adcInstance.adcValue;

            /* track how long acceleration stays at 0 */

            /* reset the conversion flag */
            adcInstance.adcConvDone = false;
            /* start the next ADC (Analog to Digital Converter) conversion using the PDB timer */
            PDB_DRV_SoftTriggerCmd(pdbInstance.instance);
        }

        txData[0] = ui8_getCurrentGear();
        uint16_t currentRpm=ui16_getRpm();
		txData[1] = (uint8_t)(currentRpm >> 8);
		txData[2] = (uint8_t) currentRpm;
		txData[3] = (uint8_t) f_getSpeed();

		if ((memcmp(previousTxData, txData, sizeof(previousTxData)) != 0)
				&& FLEXCAN_DRV_GetTransferStatus(INST_FLEXCAN, TX_MAILBOX) != STATUS_BUSY){
		   CAN_SendData(TX_MAILBOX, TRANSMISSION_TX_ID, txData, &txInfo);
		   memcpy(previousTxData, txData, sizeof(txData));
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
void v_selfTest(ftm_instanceConfig_t *ftmMotorInstance,
                ftm_instanceConfig_t *ftmServoInstance) {
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
    ftmServoInstance.ftm_stateStruct = (ftm_state_t ) { 0 };
    ftmServoInstance.hwChannelId = flexTimer_pwm_servo_IndependentChannelsConfig[0].hwChannelId;

    ftmMotorInstance.FTM_INSTANCE = INST_FLEXTIMER_PWM_MOTOR;
    ftmMotorInstance.ftm_pwmInitConfig = &flexTimer_pwm_motor_InitConfig;
    ftmMotorInstance.ftm_pwmConfig = &flexTimer_pwm_motor_PwmConfig;
    ftmMotorInstance.ftm_stateStruct = (ftm_state_t ) { 0 };
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

    INT_SYS_InstallHandler(adcInstance.irqNumber,
                           adcInstance.isrHandler,
                           (isr_t*) 0);

    v_ftmInit(&ftmServoInstance);
    v_ftmInit(&ftmMotorInstance);

    v_pdbInit(&pdbInstance);
    v_setMotorDirection(&ftmMotorInstance);

    /*CAN INIT*/

    FLEXCAN_DRV_Init(INST_FLEXCAN, &flexcanState0, &flexcanInitConfig0);
    FLEXCAN_DRV_ConfigTxMb(INST_FLEXCAN, TX_MAILBOX, &txInfo, TRANSMISSION_RX_ID);
    /* Configure RX message buffer */
    FLEXCAN_DRV_ConfigRxMb(INST_FLEXCAN, RX_MAILBOX, &rxInfo, BRAKES_TX_ID);
    /* Installs a callback function for the FlexCAN IRQ handler */
    FLEXCAN_DRV_InstallEventCallback(INST_FLEXCAN, &flexcanCallback, NULL);
    /* Start receiving CAN messages on the configured mailbox */
    FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MAILBOX, &rx_msg);

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

/* @brief: Callback for FlexCAN to parse incoming messages. */
void flexcanCallback(uint8_t instance, flexcan_event_type_t eventType,
                     uint32_t buffIdx, flexcan_state_t *flexcanState) {
    if(eventType == FLEXCAN_EVENT_RX_COMPLETE && buffIdx == RX_MAILBOX){
        /* Extract brake level (0 to 7 limit) */
        ui8BrakeLevel = rx_msg.data[0];
        if (ui8BrakeLevel > 7) ui8BrakeLevel = 7;
        /* Set up for the next reception */
        FLEXCAN_DRV_Receive(instance, RX_MAILBOX, &rx_msg);
    }
}
