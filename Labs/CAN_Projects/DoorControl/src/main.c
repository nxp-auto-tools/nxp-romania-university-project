/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 */

/*
 * TODO: recheck the exercise
 * Exercises:
 * 0. When the execution reach	return exit_code;
 * 1. Magic numbers - replace with define
 * 2. BP main blocks: init, cyclic, interrupt
 * 3. Hysteresis for cooler
 * 4. Switch motor direction when is off
 * 5. Turn the motor ON/OFF after 4x Switch pushes
 */

/* Include header files section --------------------------------------------------- */
#include "sdk_project_config.h"
#include "interrupt_manager.h"
#include "osif.h"
#include <string.h>

#include "adc_utils.h"
#include "pdb_utils.h"
#include "clock_utils.h"
#include "can_utils.h"

/* Define section ----------------------------------------------------------------- */

#define TX_MAILBOX 9

/* Define Buttons ports/pins/irq */
//TODO: add define description
#define BTN_GPIO_A PTA
#define BTN_DW_PIN 15U
#define BTN_UP_PIN 16U
#define BTN_LOCK_PIN 1U
#define BTN_CLIM_PIN 0U
#define BTN_PORT_A PORTA
#define BTN_PORTA_IRQn   PORTA_IRQn

/*ADC channel for potentiometer*/
#define ADC0_CHN        ADC_INPUTCHAN_EXT10

/*ADC channel for temperature sensor*/
#define ADC1_CHN        ADC_INPUTCHAN_EXT8

/* ADC VALUES for 12bit ADC */
#define ADCVALUE_5V_12B 4095
#define ADCVALUE_1V_12B 820

/* TEMPERATURE hysteresis threshold in celsius degrees*/
#define TEMPERATURE_THRESHOLD 0.7f

/* TEMPERATURE settings from potentiometer min/max in celsius degrees*/
#define TEMP_LOWER_SETTING 16
#define TEMP_HIGHER_SETTING 32

/* TEMPERATURE sensor range in celsius degrees */
#define SENSOR_MINIMUM_TEMPERATURE 0
#define SENSOR_MAXIMUM_TEMPERATURE 100

/* Define Stepper ports/pins/irq  */

/*   STEP   */
#define STP_DRV_STEP_PORT 	PTE
#define STP_DRV_STEP_PIN  	1

/*   EN   */
#define STP_DRV_EN_PORT 	PTE
#define STP_DRV_EN_PIN  	15

/*   DIRECTION   */
#define STP_DRV_DIR_PORT 	PTE
#define STP_DRV_DIR_PIN  	14

/* LPIT channel used */
#define LPIT_CHANNEL        0UL
#define LPIT_Channel_IRQn   LPIT0_Ch0_IRQn

/*   COOLER   */
#define COOLER_PORT 	PTC
#define COOLER_PIN  	1

/* HELPFUL MACROS */
#define SET_BIT(x, pos) (x |= (1U << pos))
#define CLEAR_BIT(x, pos) (x &= (~(1U<< pos)))
#define TOGGLE_BIT(x, pos) x ^= (1U<< pos)
#define CHECK_BIT(x, pos) (x & (1UL << pos) )

/* Active-low button read: returns true when the button is physically pressed */
#define BTN_IS_PRESSED(PORT, PIN) ( ((PORT)->PDIR & (1U << (PIN))) == 0U )

/* Variables declaration section -------------------------------------------------- */

/*Variable for storing the up/down buttons positions and child protection/climate states*/
static volatile uint8_t ui8_ButtonFlags = 0b00001100;

/* --- Flags layout --- */
enum {
    BTNFLAG_DOWN = 0, BTNFLAG_UP = 1, FLAG_LOCK = 2, /* Lock for stepper motor*/
    FLAG_CLIM = 3 /* Climate system state*/
};

static adc_instanceConfig_t adc0Instance;
static adc_instanceConfig_t adc1Instance;
static pdb_instanceConfig_t pdb0Instance;
static pdb_instanceConfig_t pdb1Instance;

static flexcan_data_info_t txInfo = { .data_length = 3U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U

};

/* Function prototype section ----------------------------------------------------- */

/* System init */
void v_Init(void);

/* Potentiometer initialization*/
void v_PotentiometerInit(void);

/* Temperature sensor initialization*/
void v_TemperatureSensorInit(void);

/* @brief: ADC Interrupt Service Routine.Read the conversion result, store it into a variable and set a specified flag.*/
void v_Adc0ISR(void);

/* @brief: ADC Interrupt Service Routine.Read the conversion result, store it into a variable and set a specified flag.*/
void v_Adc1ISR(void);

/* LPIT interrupt handler. When an interrupt occurs clear channel flag and toggle DRV step */
void v_LpitISR(void);

/* Button interrupt handler D */
void v_ButtonISR(void);

/* @brief: Installs and enables a interrupt handler. */
void v_InterruptInit(IRQn_Type irqNumber, const isr_t isrHandler);

/* Drv/stepper init function */
void v_DrvInit(void);

/* @brief: Converts the value of the potentiometer ADC into celsius degrees. */
float f_GetTemperatureFromPotentiometer(uint16_t value);

/* @brief: Converts the value of the temperature sensor ADC into celsius degrees. */
float f_GetTemperatureFromSensor(uint16_t ui16_AdcValueParam);

/* @brief: Handles the movement of the motor based on the flags*/
void v_ProcessMotorMovement(uint8_t flags);

/* main function */
int main(void) {

    /* Write your local variable definition here ----------------------- */

    float f_previousSensorTemperature = 0;
    float f_CurrentTemperature = 0;
    float f_UserTemperature = 0;
    float f_LowerBound;
    float f_UpperBound;

    uint8_t ui8_CurrentFlags = 0;

    uint8_t txData[3] = { 0 };
    uint8_t previousTxData[3] = { 0 };

    /* System initialization*/
    v_Init();

    /* Infinite loop -----------------------------------------------------------------*/
    while (true) {

        ui8_CurrentFlags = ui8_ButtonFlags;

        /* Process the motor movement based on current flags */
        v_ProcessMotorMovement(ui8_CurrentFlags);

        /* Process the result to get the value in volts */
        if (adc0Instance.adcConvDone == true) {

            /* Get user temperature setting*/
            f_UserTemperature = f_GetTemperatureFromPotentiometer(adc0Instance.adcValue);

            /* Clear conversion done interrupt flag */
            adc0Instance.adcConvDone = false;

            /* Trigger PDB timer */
            PDB_DRV_SoftTriggerCmd(pdb0Instance.instance);
        } else {
            /*do nothing*/
        }

        if (adc1Instance.adcConvDone == true) {

            /* Get current temperature setting*/
            f_CurrentTemperature = f_GetTemperatureFromSensor(adc1Instance.adcValue);

            /* Clear conversion done interrupt flag */
            adc1Instance.adcConvDone = false;

            /* Trigger PDB timer */
            PDB_DRV_SoftTriggerCmd(pdb1Instance.instance);

        } else {
            /*do nothing*/
        }

        /* Verify if climate system is on */
        if (CHECK_BIT(ui8_CurrentFlags, FLAG_CLIM)) {

            f_LowerBound = f_UserTemperature - TEMPERATURE_THRESHOLD;
            f_UpperBound = f_UserTemperature + TEMPERATURE_THRESHOLD;

            /* Verify if current temperature is higher than upper bound*/
            if (f_CurrentTemperature > f_UpperBound) {
                /*Turn ON cooler*/
                PINS_DRV_SetPins(COOLER_PORT, (1 << COOLER_PIN));
            }
            /* Verify if current temperature is lower than lower bound*/
            else if (f_CurrentTemperature < f_LowerBound) {
                /*Turn OFF cooler*/
                PINS_DRV_ClearPins(COOLER_PORT, (1 << COOLER_PIN));
            } else {
                /*If within hysteresis band, do nothing (keep previous state)*/
            }

        } else {
            /*Climate system is OFF, turn off the cooler*/
            PINS_DRV_ClearPins(COOLER_PORT, (1 << COOLER_PIN));
        }

        /* 
         * TODO 3: Populate the transmission data for the HMI project.
         * Hint: Check the HMI project to see what data it expects.
         */


        /* 
         * TODO 2: Send the CAN message with the comfort data.
         * Hint: Use the function CAN_SendData(uint8_t mailbox, uint32_t msg_id, uint8_t txData[], flexcan_data_info_t *txInfo).
         * Make sure the CAN TX message buffer is not busy before sending.
         * You can also implement a condition to send the message only if the data has changed significantly (e.g., temperature change > 0.9°C or flag changes).
         */

        f_previousSensorTemperature = f_CurrentTemperature;

    }
}

void v_Init(void) {

    /* Potentiometer initialization */
    v_PotentiometerInit();

    /* Temperature sensor initialization */
    v_TemperatureSensorInit();

    /* Clock initialization */
    CLK_v_ClockInit();

    /* Initialize pins
     *    -    See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    v_DrvInit();

    /* Stepper init*/
    PINS_DRV_SetPins(STP_DRV_EN_PORT, (1 << STP_DRV_EN_PIN));
    //TODO: double check if it is still needed
    PINS_DRV_SetPins(STP_DRV_DIR_PORT, (1 << STP_DRV_DIR_PIN));
    PINS_DRV_ClearPins(STP_DRV_DIR_PORT, (1 << STP_DRV_DIR_PIN));

    /* Buttons init*/
    PINS_DRV_SetPinIntSel(BTN_PORT_A, BTN_DW_PIN, PORT_INT_EITHER_EDGE);
    PINS_DRV_SetPinIntSel(BTN_PORT_A, BTN_UP_PIN, PORT_INT_EITHER_EDGE);
    PINS_DRV_SetPinIntSel(BTN_PORT_A, BTN_LOCK_PIN, PORT_INT_EITHER_EDGE);
    PINS_DRV_SetPinIntSel(BTN_PORT_A, BTN_CLIM_PIN, PORT_INT_EITHER_EDGE);

    /* ADC init */
    v_adcInit(&adc0Instance);
    v_adcInit(&adc1Instance);

    /* PDB init */
    v_pdbInit(&pdb0Instance);
    v_pdbInit(&pdb1Instance);

    /*
     * TODO 0: Initialize the FlexCAN module.
     * Hint: Use the function FLEXCAN_DRV_Init(uint8_t instance, flexcan_state_t *state, const flexcan_user_config_t *config).
     * You should use 'INST_FLEXCAN', '&flexcanState0', and '&flexcanInitConfig0'.
     */

    /* 
     * TODO 1: Configure the TX message buffer for sending comfort data.
     * Hint: Use the function FLEXCAN_DRV_ConfigTxMb(instance, mb_idx, &tx_info, msg_id).
     */


    /* Interrupt init */
    v_InterruptInit(adc0Instance.irqNumber, adc0Instance.isrHandler);
    v_InterruptInit(adc1Instance.irqNumber, adc1Instance.isrHandler);
    v_InterruptInit(BTN_PORTA_IRQn, &v_ButtonISR);
}

/* Potentiometer initialization */
void v_PotentiometerInit(void) {

    adc0Instance.adcConvDone = false;
    adc0Instance.adcValue = 0;
    adc0Instance.chanIndex = 0;
    adc0Instance.inputChan = ADC0_CHN;
    adc0Instance.channelConfig = &ADC_0_ChnConfig0;
    adc0Instance.converterConfig = &ADC_0_ConvConfig0;
    adc0Instance.instance = INST_ADC_0;
    adc0Instance.irqNumber = ADC0_IRQn;
    adc0Instance.isrHandler = &v_Adc0ISR;

    pdb0Instance.adcTrigConfig = &pdb_0_adcTrigConfig0;
    pdb0Instance.chn = 0U;
    pdb0Instance.instance = INST_PDB_0;
    pdb0Instance.pdlyTimeout_microS = PDLY_TIMEOUT;
    pdb0Instance.preChn = 0U;
    pdb0Instance.timerConfig = &pdb_0_timerConfig0;
}

/* Temperature Sensor Initialization*/
void v_TemperatureSensorInit(void) {

    adc1Instance.adcConvDone = false;
    adc1Instance.adcValue = 0;
    adc1Instance.chanIndex = 0;
    adc1Instance.channelConfig = &ADC_1_ChnConfig0;
    adc1Instance.converterConfig = &ADC_1_ConvConfig0;
    adc1Instance.inputChan = ADC1_CHN;
    adc1Instance.instance = INST_ADC_1;
    adc1Instance.irqNumber = ADC1_IRQn;
    adc1Instance.isrHandler = &v_Adc1ISR;

    pdb1Instance.adcTrigConfig = &pdb_1_adcTrigConfig0;
    pdb1Instance.chn = 0U;
    pdb1Instance.instance = INST_PDB_1;
    pdb1Instance.pdlyTimeout_microS = PDLY_TIMEOUT;
    pdb1Instance.preChn = 0U;
    pdb1Instance.timerConfig = &pdb_1_timerConfig0;

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
void v_InterruptInit(IRQn_Type irqNumber, const isr_t isrHandler) {

    INT_SYS_InstallHandler(irqNumber, isrHandler, (isr_t*) 0);

    /* Enable ADC 1 interrupt */
    INT_SYS_EnableIRQ(irqNumber);

}

float f_GetTemperatureFromSensor(uint16_t ui16_AdcValueParam) {

    const uint16_t cui16_AdcMax = ADCVALUE_1V_12B;

    const uint16_t cui16_TempMin = SENSOR_MINIMUM_TEMPERATURE;
    const uint16_t cui16_TempMax = SENSOR_MAXIMUM_TEMPERATURE;

    float f_ReturnValue;

    /* clamp to max */
    if (ui16_AdcValueParam > cui16_AdcMax) {
        ui16_AdcValueParam = cui16_AdcMax;
    } else {
        /* do nothing */
    }

    f_ReturnValue = (float)cui16_TempMin
        + ((float)(ui16_AdcValueParam * (cui16_TempMax - cui16_TempMin)) /(float) cui16_AdcMax);

    return f_ReturnValue;
}

float f_GetTemperatureFromPotentiometer(uint16_t ui16_AdcValueParam) {

    const uint16_t cui16_AdcMax = ADCVALUE_5V_12B;

    const uint16_t cui16_TempMin = TEMP_LOWER_SETTING;
    const uint16_t cui16_TempMax = TEMP_HIGHER_SETTING;

    float f_ReturnValue;

    /* clamp to max */
    if (ui16_AdcValueParam > cui16_AdcMax) {
        ui16_AdcValueParam = cui16_AdcMax;
    } else {
        /* do nothing */
    }

    f_ReturnValue = cui16_TempMin
        + ((ui16_AdcValueParam * (cui16_TempMax - cui16_TempMin)) / cui16_AdcMax);

    return f_ReturnValue;
}

/* @brief: Handles the movement of the motor based on the flags*/
void v_ProcessMotorMovement(uint8_t flags) {
    /* Check if child lock is not active*/
    if (CHECK_BIT(flags, FLAG_LOCK)) {
        /* --- Motor control based on continuous press --- */
        if (CHECK_BIT(flags, BTNFLAG_DOWN)) {
            /* DOWN pressed: set direction DOWN & enable driver */
            PINS_DRV_ClearPins(STP_DRV_DIR_PORT, (1U << STP_DRV_DIR_PIN));
            PINS_DRV_ClearPins(STP_DRV_EN_PORT, (1U << STP_DRV_EN_PIN));
        } else if (CHECK_BIT(flags, BTNFLAG_UP)) {
            /* UP pressed: set direction UP & enable driver */
            PINS_DRV_SetPins(STP_DRV_DIR_PORT, (1U << STP_DRV_DIR_PIN));
            PINS_DRV_ClearPins(STP_DRV_EN_PORT, (1U << STP_DRV_EN_PIN));
        } else {
            /* No button pressed,disable driver.*/
            PINS_DRV_SetPins(STP_DRV_EN_PORT, (1U << STP_DRV_EN_PIN));
        }
    } else {
        /* Child lock active: stop motor */
        PINS_DRV_SetPins(STP_DRV_EN_PORT, (1U << STP_DRV_EN_PIN));
    }
}

/*DRV init function */
void v_DrvInit(void) {

    /* Initialize LPIT instance 0
     *  -   Reset and enable peripheral
     */
    LPIT_DRV_Init(INST_LPIT_CONFIG_1, &lpit1_InitConfig);

    INT_SYS_InstallHandler(LPIT_Channel_IRQn, &v_LpitISR, (isr_t*) 0);
    /* Initialize LPIT channel 0 and configure it as a periodic counter
     * which is used to generate an interrupt every second.
     */
    LPIT_DRV_InitChannel(INST_LPIT_CONFIG_1, LPIT_CHANNEL, &lpit1_ChnConfig0);

    /* Start LPIT0 channel 0 counter */
    LPIT_DRV_StartTimerChannels(INST_LPIT_CONFIG_1, (1 << LPIT_CHANNEL));
}

/* @brief: ADC Interrupt Service Routine.
 *        Read the conversion result, store it
 *        into a variable and set a specified flag.
 */
void v_Adc0ISR(void) {
    /* Get channel result from ADC channel */
    ADC_DRV_GetChanResult(adc0Instance.instance,
                          adc0Instance.chanIndex,
                          (uint16_t*) &adc0Instance.adcValue);
    /* Set ADC conversion complete flag */
    adc0Instance.adcConvDone = true;
}

/* @brief: ADC Interrupt Service Routine.
 *        Read the conversion result, store it
 *        into a variable and set a specified flag.
 */
void v_Adc1ISR(void) {
    /* Get channel result from ADC channel */
    ADC_DRV_GetChanResult(adc1Instance.instance,
                          adc1Instance.chanIndex,
                          (uint16_t*) &adc1Instance.adcValue); //Trebe regandit isr-u
    /* Set ADC conversion complete flag */
    adc1Instance.adcConvDone = true;
}

/*
 * @brief: LPIT interrupt handler.
 *         When an interrupt occurs clear channel flag and toggle DRV step
 */
void v_LpitISR(void) {
    if (LPIT_DRV_GetInterruptFlagTimerChannels(INST_LPIT_CONFIG_1,
                                               (1 << LPIT_CHANNEL))) {
        /* Clear LPIT channel flag */
        LPIT_DRV_ClearInterruptFlagTimerChannels(INST_LPIT_CONFIG_1,
                                                 (1 << LPIT_CHANNEL));

        /* Toggle driver step*/
        PINS_DRV_TogglePins(STP_DRV_STEP_PORT,
                            (uint32_t) (1u << STP_DRV_STEP_PIN));
    } else {
        /* do nothing */
    }
}

/**
 * @brief Interrupt Service Routine for button inputs.
 *
 * This function handles GPIO interrupts triggered by button presses on port A.
 * It checks which button caused the interrupt (DOWN, UP, LOCK, CLIM) and performs
 * corresponding actions such as toggling motor direction, enabling/disabling the
 * stepper driver, controlling the cooler, and updating system states like `locked`
 * and `climateSystemState`. After processing, it clears the interrupt flags for each pin.
 */
void v_ButtonISR(void) {
    uint32_t ui32_IntFlags = PINS_DRV_GetPortIntFlag(BTN_PORT_A);

    /* DOWN - check if the interrupt is from the DOWN button*/
    if (ui32_IntFlags & (1U << BTN_DW_PIN)) {
        if (BTN_IS_PRESSED(PTA, BTN_DW_PIN)) {
            SET_BIT(ui8_ButtonFlags, BTNFLAG_DOWN); /* pressed */
        } else {
            CLEAR_BIT(ui8_ButtonFlags, BTNFLAG_DOWN); /* released */
        }
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_A, BTN_DW_PIN);
    }

    /* UP - check if the interrupt is from the UP button */
    if (ui32_IntFlags & (1U << BTN_UP_PIN)) {
        if (BTN_IS_PRESSED(PTA, BTN_UP_PIN)) {
            SET_BIT(ui8_ButtonFlags, BTNFLAG_UP); /* pressed */
        } else {
            CLEAR_BIT(ui8_ButtonFlags, BTNFLAG_UP);/* released */
        }
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_A, BTN_UP_PIN);
    }

    /* LOCK - check if the interrupt is from the LOCK button */
    if (ui32_IntFlags & (1U << BTN_LOCK_PIN)) {
        if (BTN_IS_PRESSED(PTA, BTN_LOCK_PIN)) {
            TOGGLE_BIT(ui8_ButtonFlags, FLAG_LOCK);/* pressed */
        } else {
            /* released */
        }
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_A, BTN_LOCK_PIN);
    }

    /* CLIMATE ON/OF - check if the interrupt is from the DOWN buttonF */
    if (ui32_IntFlags & (1U << BTN_CLIM_PIN)) {
        if (BTN_IS_PRESSED(PTA, BTN_CLIM_PIN)) {
            TOGGLE_BIT(ui8_ButtonFlags, FLAG_CLIM);/* pressed */
        } else {
            /* released - do nothing*/
        }
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_A, BTN_CLIM_PIN);
    }
}
