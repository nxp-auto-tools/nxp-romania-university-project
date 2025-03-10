/*
 * Copyright 2020, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Including necessary configuration files. */
#include "sdk_project_config.h"
#include "interrupt_manager.h"
#include "osif.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "peripherals_pdb_1.h"

/********* DEFINE *******************/
#define PCC_CLOCK			PCC_PORTD_CLOCK
#define MOTOR_DIR_PORT		PTE
#define MOTOR_DIR_PIN		14
#define MOTOR_IN_PORT		PTB
#define MOTOR_IN_PIN		13
#define MOTOR_PWM_CHANNEL	1				// we have configured the flex timer to use channel 1

#define ADC_INSTANCE    1UL
#define ADC_CHN         ADC_INPUTCHAN_EXT12
#define MAX_ACC			100

#define PTE0 0
#define PDB_INSTANCE    1UL

/* Timeout in ms for blocking operations on LPUART */
#define TIMEOUT     500U

/* Timeout for PDB in microseconds */
#define PDLY_TIMEOUT   10000UL

#define MIN_DUTY_CYCLE 50
#define MAX_DUTY_CYCLE 1800
#define DUTY_CYCLE_TO_VOLTS_SCALE 1.25f
#define RPM_MAX_DUTY_CYCLE 0x8000	// 32768

/* GEARBOX configuration */
#define MAX_GEAR 6
#define MIN_GEAR 1
#define MAX_SPEED 215 // Maximum speed in km/h
#define MAX_RPM 7000  // Maximum RPM
#define IDLE_RPM 800  // Idle RPM
#define GEAR_CHANGE_THRESHOLD 5 // Threshold for smooth gear changes
#define ACC_ZERO_COUNTER_THRESHOLD 10 // if acceleration is read as 0 X times then turn off motor

// Speed thresholds for each gear
const int gearSpeed[MAX_GEAR + 1] = { 3, 20, 40, 70, 110, 160, 200 };
const float gearRatios[MAX_GEAR + 1] = { 0, 10, 7, 5, 3, 2, 1.5 }; // Approximate gear ratios

/* ******** VARIABLES ***************** */
volatile int exitCode = 0;

volatile int servoDutyCycle = 0U;   // Duty cycle needed by the servo to display the current gear
volatile int currentGear = 1U;      // Current gear (Displayed by the servo position)
float speed = 0.0;			        // Current speed (Not showed anywhere. For now...)
int motorDutyCycle = 0U;            // Duty cycle needed by the motor to display the current RPM
int rpm = IDLE_RPM;					// Current RPM
volatile int acceleration = 0U;		// Current acceleration level 0 - 100%
int accelerationZeroCounter = 0U;
/* Flag used to store if an ADC IRQ was executed */
volatile bool adcConvDone;
/* Variable to store value from ADC conversion */
volatile uint16_t adcRawValue;

/*  ***** FUNCTIONS  ******** */

/* @brief: ADC Interrupt Service Routine.
 *        Read the conversion result, store it
 *        into a variable and set a specified flag.
 */
void ADC_IRQHandler(void) {
    /* Get channel result from ADC channel */
    ADC_DRV_GetChanResult(ADC_INSTANCE, 0U, (uint16_t*) &adcRawValue);
    /* Set ADC conversion complete flag */
    adcConvDone = true;
}

/* @brief: Calculate the values to be used by pdb to generate
 *        a interrupt at a specific timeout.
 * @param pdbConfig: pointer to the PDB configuration struct
 * @param type:      pdb_timer_config_t *
 * @param uSec:      interval for pdb interrupt in microseconds
 * @param type:      uint32_t
 * @param intVal:    pointer to the storage element where to set the calculated value
 * @param type:      uint16_t
 * @return:          Returns true if the interrupt period can be achieved, false if not
 * @return type:     bool
 */
bool calculateIntValue(const pdb_timer_config_t *pdbConfig, uint32_t uSec,
        uint16_t *intVal) {
    /* Local variables used to store different parameters
     * such as frequency and prescalers
     */
    uint32_t intVal_l = 0;
    uint8_t pdbPrescaler = (1 << pdbConfig->clkPreDiv);
    uint8_t pdbPrescalerMult = 0;
    uint32_t pdbFrequency;

    bool resultValid = false;

    /* Get the Prescaler Multiplier from the configuration structure */
    switch (pdbConfig->clkPreMultFactor) {
    case PDB_CLK_PREMULT_FACT_AS_1:
        pdbPrescalerMult = 1U;
        break;
    case PDB_CLK_PREMULT_FACT_AS_10:
        pdbPrescalerMult = 10U;
        break;
    case PDB_CLK_PREMULT_FACT_AS_20:
        pdbPrescalerMult = 20U;
        break;
    case PDB_CLK_PREMULT_FACT_AS_40:
        pdbPrescalerMult = 40U;
        break;
    default:
        /* Defaulting the multiplier to 1 to avoid dividing by 0*/
        pdbPrescalerMult = 1U;
        break;
    }

    /* Get the frequency of the PDB clock source and scale it
     * so that the result will be in microseconds.
     */
    CLOCK_SYS_GetFreq(CORE_CLOCK, &pdbFrequency);
    pdbFrequency /= 1000000;

    /* Calculate the interrupt value for the prescaler, multiplier, frequency
     * configured and time needed.
     */
    intVal_l = (pdbFrequency * uSec) / (pdbPrescaler * pdbPrescalerMult);

    /* Check if the value belongs to the interval */
    if ((intVal_l == 0) || (intVal_l >= (1 << 16))) {
        resultValid = false;
        (*intVal) = 0U;
    } else {
        resultValid = true;
        (*intVal) = (uint16_t) intVal_l;
    }

    return resultValid;
}

/* set delay in cycles */
void delay(volatile int cycles) {
    /* Delay function - do nothing for a number of cycles */
    while (cycles--)
        ;
}

// Function to calculate RPM based on speed, gear, and acceleration
void calculateRPM() {
    // RPM is affected by speed, gear ratio, acceleration, and a factor for slow RPM increase at high speed
    // TODO: Implement the way RPM is calculated

}

void displayRPM() {
    /* The motor has 2 pins for direction: In1 and In2
     * To set the motor forward we need to set In1 to high and In2 to low
     * Since the pin In2 is not connected it is considered as low.
     * We need to set In1 to high in order to have a valid direction.*/
    // TODO: Set the Motor direction pin to 1
    MOTOR_DIR_PORT->PSOR = (1 << MOTOR_DIR_PIN);

    /* Duty cycle percent 0-0x8000 -> 0-100%*/
    // TODO: Transform from RPM to duty cycle

    // Send Servo command
    FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_MOTOR,
            flexTimer_pwm_motor_IndependentChannelsConfig[MOTOR_PWM_CHANNEL].hwChannelId,
            FTM_PWM_UPDATE_IN_DUTY_CYCLE, (uint16_t) motorDutyCycle, 0U, true);

}

// Function to update speed based on acceleration and gear
void updateSpeed() {
    // TODO: Implement the way RPM is calculated
}

// Function to update gear based on speed with smooth transitions
void updateGear() {
    // TODO: Gear change logic.
}

/* @brief: Calculate the values to be sent to the Servo motor in order to indicate
 * the current gear:
 *	Ex: for 6 gears display:
 * 								3rd gear
 * 							2nd gear ^  4th gear
 * 						1st gear     |     5th gear
 * No acceleration (motor off)       |        6th gear
 *
 * Hint: At motor off servoDutyCycle is MIN_DUTY_CYCLE
 *
 */
void displayGear() {
    /* TODO set servoDutyCycle to the correct value so it indicates the correct position of the speed */

    // Send Servo command
    FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_SERVO,
            flexTimer_pwm_servo_IndependentChannelsConfig[0].hwChannelId,
            FTM_PWM_UPDATE_IN_TICKS, (uint16_t) servoDutyCycle, 0U, true);

}

/* main function */
int main(void) {
    uint16_t delayValue = 0;
    /* Write your local variable definition here */
    ftm_state_t ftmStateStructMotor;
    ftm_state_t ftmStateStructServo;

    /* Variables in which we store data from ADC */

    uint16_t adcMax;

    IRQn_Type adcIRQ;

    adcConvDone = false;

    /* Initialize clock module */
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
            g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    /* Initialize pins
     *    -    See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    /* Get ADC max value from the resolution */
    adcMax = (uint16_t) (1 << 12);
//	adcMax = 3904; // max value read with the debugger

    /* Configure and calibrate the ADC converter
     *  -   See ADC component for the configuration details
     */
    DEV_ASSERT(ADC_0_ChnConfig0.channel == ADC_CHN);

    ADC_DRV_ConfigConverter(ADC_INSTANCE, &ADC_0_ConvConfig0);
    ADC_DRV_AutoCalibration(ADC_INSTANCE);
    ADC_DRV_ConfigChan(ADC_INSTANCE, 0UL, &ADC_0_ChnConfig0);

    /* Initialize FTM instance - Motor*/
    FTM_DRV_Init(INST_FLEXTIMER_PWM_MOTOR, &flexTimer_pwm_motor_InitConfig,
            &ftmStateStructMotor);

    /* Initialize FTM instance - Servo*/
    FTM_DRV_Init(INST_FLEXTIMER_PWM_SERVO, &flexTimer_pwm_servo_InitConfig,
            &ftmStateStructServo);

    /* Initialize FTM PWM - Motor*/
    FTM_DRV_InitPwm(INST_FLEXTIMER_PWM_MOTOR, &flexTimer_pwm_motor_PwmConfig);

    /* Initialize FTM PWM - Servo*/
    FTM_DRV_InitPwm(INST_FLEXTIMER_PWM_SERVO, &flexTimer_pwm_servo_PwmConfig);

    switch (ADC_INSTANCE) {
    case 0UL:
        adcIRQ = ADC0_IRQn;
        break;
    case 1UL:
        adcIRQ = ADC1_IRQn;
        break;
    default:
        adcIRQ = ADC1_IRQn;
        break;
    }

    INT_SYS_InstallHandler(adcIRQ, &ADC_IRQHandler, (isr_t*) 0);

    /* Calculate the value needed for PDB instance
     * to generate an interrupt at a specified timeout.
     * If the value can not be reached, stop the application flow
     */
    if (!calculateIntValue(&pdb_1_timerConfig0, PDLY_TIMEOUT, &delayValue)) {
        /* Stop the application flow */
        while (1)
            ;
    }

    /* Setup PDB instance
     *  -   See PDB component for details
     *  Note: Pre multiplier and Prescaler values come from
     *        calculateIntValue function.
     */
    PDB_DRV_Init(PDB_INSTANCE, &pdb_1_timerConfig0);
    PDB_DRV_Enable(PDB_INSTANCE);
    PDB_DRV_ConfigAdcPreTrigger(PDB_INSTANCE, 0UL, &pdb_1_adcTrigConfig0);
    PDB_DRV_SetTimerModulusValue(PDB_INSTANCE, (uint32_t) delayValue);
    PDB_DRV_SetAdcPreTriggerDelayValue(PDB_INSTANCE, 0UL, 0UL,
            (uint32_t) delayValue);
    PDB_DRV_LoadValuesCmd(PDB_INSTANCE);
    PDB_DRV_SoftTriggerCmd(PDB_INSTANCE);

    /* Enable ADC 1 interrupt */
    INT_SYS_EnableIRQ(adcIRQ);

    // Start position for servo
    // Send Servo command
    FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_SERVO,
            flexTimer_pwm_servo_IndependentChannelsConfig[0].hwChannelId,
            FTM_PWM_UPDATE_IN_TICKS, (uint16_t) MAX_DUTY_CYCLE, 0U, true);
    // wait to start the engine
    delay(10000000);

    /* Infinite loop */
    while (1) {
        // update the speed of the car
        updateSpeed();
        // switch gear if needed
        updateGear();
        // set the servo to indicate the current gear
        displayGear();
        // update motor RPM
        calculateRPM();
        // send the command to the driver
        displayRPM();

        OSIF_TimeDelay(10);

        /* Process the result to get the value for acceleration */
        if (adcConvDone == true) {
            /* TODO: Process the result to get the acceleration in 0-100 */

            /* Clear conversion done interrupt flag */
            adcConvDone = false;
            /* Trigger PDB timer */
            PDB_DRV_SoftTriggerCmd(PDB_INSTANCE);
        }

        delay(500000);
    }

    return exitCode;
}
