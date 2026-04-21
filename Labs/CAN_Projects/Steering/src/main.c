/*
 * Copyright 2020, 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include header files section -------------------------------------------------------------*/
#include "sdk_project_config.h"
#include "interrupt_manager.h"
#include "osif.h"
#include <string.h>

#include "adc_utils.h"
#include "ftm_utils.h"
#include "pdb_utils.h"
#include "clock_utils.h"
#include "can_utils.h"

/* Define section ---------------------------------------------------------------------------*/
#define TX_MAILBOX 9

#define VREFH               5.0f
#define VREFL               0.0f
#define VOLTAGE_RANGE_mV       ((VREFH-VREFL)*1000)

/* PWM duty cycle limits in ticks
 * PWM is used for controlling the servo motor
 *
 * The servomotor operates between 0.4-2.4 ms dutycycle(0-180 degree turn)*/
#define MIN_DUTY_CYCLE 300  /*(300 ticks * 20 ms)/15000 ticks = 0.4 ms dutycycle*/
#define MAX_DUTY_CYCLE 1800 /*(1800 ticks * 20 ms)/15000 ticks = 2.4 ms dutycycle*/

#define FTM_UPDATE_DELAY_mS 10
#define SELFT_TEST_DELAY_mS 500

/* Variables declaration section ----------------------------------------------------------- */

static adc_instanceConfig_t adcInstance;
static ftm_instanceConfig_t ftmServoInstance;
static pdb_instanceConfig_t pdbInstance;

static flexcan_data_info_t txInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U

};
/* Function prototype section ---------------------------------------------------------------*/

/* @brief:  Initialization function for all used modules.*/
void v_init(void);

/* @brief: Installs and enables a interrupt handler. */
void v_interruptInit(IRQn_Type irqNumber, const isr_t isrHandler);

/* @brief:  Self test function. Moves the servo head to the min and max positions.*/
void v_selfTest(void);

/* @brief: Read the conversion result, store it into a variable and set a specified flag. */
void v_ADC_IRQHandler(void);

/* main function */
int main(void) {
    /* PWM duty cycle */
    uint16_t iServoDutyCycle = 0U;
    uint8_t txData[1] = { 0 };
    uint8_t previousTxData[1] = { 0 };
    v_init();

    v_selfTest();

    /* Infinite loop ------------------------------------------------------------------------*/
    while (1) {
        /*conversion from the ADC resolution to ftm_pwm clock resolution.*/
        iServoDutyCycle = ui16_scaleToDutyCycle(adcInstance.adcValue,
                                                MAX_DUTY_CYCLE,
                                                MIN_DUTY_CYCLE,
                                                (uint16_t) (VOLTAGE_RANGE_mV));

        FTM_DRV_UpdatePwmChannel(ftmServoInstance.instance,
                                 ftmServoInstance.hwChannelId,
                                 FTM_PWM_UPDATE_IN_TICKS,
                                 (uint16_t) iServoDutyCycle,
                                 0U,
                                 true);

        OSIF_TimeDelay(FTM_UPDATE_DELAY_mS);

        if (adcInstance.adcConvDone == true) {

            /*Scale the adc value to voltage range in mV*/
            v_scaleAdcValue((uint16_t) (VOLTAGE_RANGE_mV),
                            &adcInstance.adcValue);

            /* 
             * TODO 2: Populate the transmission data for the HMI project.
             * Hint: The HMI project expects the steering angle (0-180) in 'txData[0]'.
             * You can calculate this by scaling the ADC value.
             */

            /* 
             * TODO 3: Send the CAN message with the steering angle data.
             * Hint: Use the function CAN_SendData(uint8_t mailbox, uint32_t msg_id, uint8_t txData[], flexcan_data_info_t *txInfo).
             * Make sure the CAN TX message buffer is not busy before sending.
             */

            /* Trigger PDB timer*/
            PDB_DRV_SoftTriggerCmd(pdbInstance.instance);

            /* Clear conversion done interrupt flag */
            adcInstance.adcConvDone = false;
        }
    }
}

/* Functions implementation section --------------------------------------------------------------*/

/* @brief: Initialization function for all used modules.*/
void v_init(void) {

    /*Module instanceConfig instantiation*/
    adcInstance.instance = INST_ADC_1;
    adcInstance.chanIndex = 0U;
    adcInstance.channelConfig = &adc_1_ChnConfig0;
    adcInstance.converterConfig = &adc_1_ConvConfig0;
    adcInstance.adcConvDone = false;
    adcInstance.adcValue = 0;
    adcInstance.irqNumber = ADC1_IRQn;
    adcInstance.isrHandler = &v_ADC_IRQHandler;

    ftmServoInstance.instance = INST_FLEXTIMER_PWM_SERVO;
    ftmServoInstance.ftm_pwmInitConfig = &flexTimer_pwm_servo_InitConfig;
    ftmServoInstance.ftm_pwmConfig = &flexTimer_pwm_servo_PwmConfig;
    ftmServoInstance.ftm_stateStruct = (ftm_state_t ) { 0 };
    ftmServoInstance.hwChannelId = flexTimer_pwm_servo_IndependentChannelsConfig[0].hwChannelId;

    pdbInstance.instance = INST_PDB_1;
    pdbInstance.timerConfig = &pdb_1_timerConfig0;
    pdbInstance.pdlyTimeout_microS = PDLY_TIMEOUT;
    pdbInstance.chn = 0U;
    pdbInstance.preChn = 0U;
    pdbInstance.adcTrigConfig = &pdb_1_adcTrigConfig0;

    /* Initialize clock module. See clock_utils for more details */
    v_clockInit();

    /* Initialize pins -  See PinSettings component for more info */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    /*Initialize ADC module. See adc_utils for more details.
     * Implements: requirement SW_ST_005
     */
    v_adcInit(&adcInstance);

    /*Initialize FTM module. See ftm_utils for more details.
     * Implements: requirement SW_ST_004
     */
    v_ftmInit(&ftmServoInstance);

    /*Initialize PDB module. See pdb_utils for more details.
     * Implements: requirement SW_ST_006
     */
    v_pdbInit(&pdbInstance);

    /*
     * TODO 0: Initialize the FlexCAN module.
     * Hint: Use the function FLEXCAN_DRV_Init(uint8_t instance, flexcan_state_t *state, const flexcan_user_config_t *config).
     * You should use 'INST_FLEXCAN', '&flexcanState0', and '&flexcanInitConfig0'.
     */

    /* 
     * TODO 1: Configure the TX message buffer for sending steering data.
     * Hint: Use the function FLEXCAN_DRV_ConfigTxMb(instance, mb_idx, &tx_info, msg_id).
     */


    /*Initialize adc interrupt. See interrupt_utils for more details.*/
    v_interruptInit(adcInstance.irqNumber, adcInstance.isrHandler);

}
/* @brief: Installs and enables a interrupt handler.
 *
 * @param irqNumber:    The number of the interrupt. Found in S32K144.h/documentation
 * @param type:         IRQn_Type
 * @param isrHandler:   Address of the handler function to be used on interrupt
 * @param type:         isr_t
 * @return:             nothing
 * @return type:        void
 */
void v_interruptInit(IRQn_Type irqNumber, const isr_t isrHandler) {

    INT_SYS_InstallHandler(irqNumber, isrHandler, (isr_t*) 0);

    /* Enable ADC 1 interrupt */
    INT_SYS_EnableIRQ(irqNumber);

}

/* @brief:  Self test function.
 * Moves the servo head to the min and max positions
 * Implements requirement SW_ST_007*/
void v_selfTest(void) {
    /*Move servo to one side*/
    FTM_DRV_UpdatePwmChannel(ftmServoInstance.instance,
                             ftmServoInstance.hwChannelId,
                             FTM_PWM_UPDATE_IN_TICKS,
                             (uint16_t) MAX_DUTY_CYCLE,
                             0U,
                             true);

    OSIF_TimeDelay(SELFT_TEST_DELAY_mS);

    /*Move servo to the other side*/
    FTM_DRV_UpdatePwmChannel(ftmServoInstance.instance,
                             ftmServoInstance.hwChannelId,
                             FTM_PWM_UPDATE_IN_TICKS,
                             (uint16_t) MIN_DUTY_CYCLE,
                             0U,
                             true);

    OSIF_TimeDelay(SELFT_TEST_DELAY_mS);
}

/* @brief: ADC Interrupt Service Routine.
 * Read the conversion result, store it into a variable and set a specified flag.
 *
 */
void v_ADC_IRQHandler(void) {
    ADC_DRV_GetChanResult(adcInstance.instance,
                          adcInstance.chanIndex,
                          (uint16_t*) &adcInstance.adcValue);

    adcInstance.adcConvDone = true;
}
