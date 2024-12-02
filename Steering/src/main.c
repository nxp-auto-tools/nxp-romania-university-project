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
#include "peripherals_pdb_1.h"

/********* DEFINE *******************/
#define PCC_CLOCK	PCC_PORTD_CLOCK

#define ADC_INSTANCE    1UL
#define ADC_CHN         ADC_INPUTCHAN_EXT12
#define ADC_VREFH       5.0f
#define ADC_VREFL       0.0f

#define PTE0 0
#define PDB_INSTANCE    1UL

/* Timeout in ms for blocking operations on LPUART */
#define TIMEOUT     500U

/* Timeout for PDB in microseconds */
#define PDLY_TIMEOUT   10000UL

#define MIN_DUTY_CYCLE 300
#define MIN_DUTY_CYCLE 1800

/* ******** VARIABLES ***************** */
volatile int exit_code = 0;

/* PWM duty cycle */
volatile int Servo_dutyCycle = 0U;

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
	/*
	 * TODO 1: Get channel result from ADC channel
	 * 	- see ADC_DRV_GetChanResult
	 * TODO 2: Set ADC conversion complete flag
	 */

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

/* main function */
int main(void) {
	uint16_t delayValue = 0;
	/* Write your local variable definition here */
	ftm_state_t ftmStateStruct;

	/* Variables in which we store data from ADC */

	uint16_t adcMax;
	uint16_t adcValue = 0;

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

	/* Configure and calibrate the ADC converter
	 *  -   See ADC component for the configuration details
	 */
	DEV_ASSERT(ADC_0_ChnConfig0.channel == ADC_CHN);

	ADC_DRV_ConfigConverter(ADC_INSTANCE, &ADC_0_ConvConfig0);
	ADC_DRV_AutoCalibration(ADC_INSTANCE);
	ADC_DRV_ConfigChan(ADC_INSTANCE, 0UL, &ADC_0_ChnConfig0);

	/* Initialize FTM instance */
	FTM_DRV_Init(INST_FLEXTIMER_PWM_2, &flexTimer_pwm_2_InitConfig,
			&ftmStateStruct);

	/* Initialize FTM PWM */
	FTM_DRV_InitPwm(INST_FLEXTIMER_PWM_2, &flexTimer_pwm_2_PwmConfig);

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

	/* Infinite loop */
	while (1) {
		/*
		 * TODO 1: Transform ADC value into duty cycle
		 */

		/*
		 * TODO 2: Check if the duty cycle is within range
		 * 	- see MIN_DUTY_CYCLE and MAX_DUTY_CYCLE
		 */

		/*
		 * TODO 3: Update servo motor with the extracted duty cycle
		 * 	- see FTM_DRV_UpdatePwmChannel
		 */

		/*
		 * TODO 4: Checks if the conversion is ready,
		 * processes the raw value and trigger new conversion
		 * 	- see PDB_DRV_SoftTriggerCmd
		 */

	}

	return exit_code;
}
