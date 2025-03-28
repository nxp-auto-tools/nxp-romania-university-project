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
 * Exercises:
 * 0. When the execution reach	return exit_code;
 * 1. Magic numbers - replace with define
 * 2. BP main blocks: init, cyclic, interrupt
 * 3. Hysteresis for cooler
 * 4. Switch motor direction when is off
 * 5. Turn the motor ON/OFF after 4x Switch pushes
 */


/* Include header files section -------------------------*/
#include "sdk_project_config.h"
#include "interrupt_manager.h"
#include "osif.h"
#include "peripherals_pdb_1.h"

/* Define section ----------------------------------------*/

/* Define Buttons ports/pins/irq */
#define BTN_GPIO        PTC
#define BTN1_PIN        13U
#define BTN2_PIN        12U
#define BTN_PORT        PORTC
#define BTN_PORT_IRQn   PORTC_IRQn

/* Define ADC - ADC0 SE12 */
#define ADC_INSTANCE    0UL
#define ADC_CHN         ADC_INPUTCHAN_EXT12
#define ADC_VREFH       5.0f
#define ADC_VREFL       0.0f


/* Define PDB instance  */
#define PDB_INSTANCE    0UL


/* Define Stepper ports/pins/irq  */
/*   STEP   */
#define STP_DRV_STEP_PORT 	PTE
#define STP_DRV_STEP_PIN  	1  //1
/*   EN   */
#define STP_DRV_EN_PORT 	PTE //PTD
#define STP_DRV_EN_PIN  	15  //7
/*   DIRECTION   */
#define STP_DRV_DIR_PORT 	PTE
#define STP_DRV_DIR_PIN  	14


/*   COOLER   */
#define COOLER_PORT 	PTD
#define COOLER_PIN  	7


/* LPIT channel used */
#define LPIT_CHANNEL        0UL
#define LPIT_Channel_IRQn   LPIT0_Ch0_IRQn

/* Timeout for PDB in microseconds */
#define PDLY_TIMEOUT   10000UL //1000000UL


/* Variables declaration section ------------------------------- */

volatile int exit_code = 0;

/* Flag used to store if an ADC IRQ was executed */
volatile bool adcConvDone;
/* Variable to store value from ADC conversion */
volatile uint16_t adcRawValue;

/* Delay used for PDB calculation */
uint16_t delayValue = 0;


/* Function prototype section ---------------------------------------*/

/* @brief: ADC Interrupt Service Routine.Read the conversion result, store it into a variable and set a specified flag.*/
void v_AdcISR(void);

/* LPIT interrupt handler. When an interrupt occurs clear channel flag and toggle DRV step */
void v_LpitISR(void);

/* Button interrupt handler D */
void v_ButtonISR(void);

/* @brief: Calculate the values to be used by pdb to generate a interrupt at a specific timeout. */
bool calculateIntValue(const pdb_timer_config_t *pdbConfig, uint32_t uSec, uint16_t * intVal);

/* Init functions declaration */
/* Clock system init function*/
void v_SystemInit(void);

/* Drv init function */
void v_DrvInit(void);

/*Button init function */
void v_ButtonInit(void);

/*ADC init function*/
void v_AdcInit(void);



/* main function */
int main(void)
{
	/* Write your local variable definition here ----------------------- */


	/* Variables in which we store data from ADC */
	uint16_t adcMax;
	uint16_t adcValue=0;

	/*Initialize the adc convertor */
    adcConvDone = false;
	/* Get ADC max value from the resolution */
	adcMax = (uint16_t) (1 << 12);

    /*Call system init function*/
	v_SystemInit();

	v_DrvInit();

	v_ButtonInit();

    v_AdcInit();


    /* Infinite loop -----------------------------------------------------------------*/
	while (1)
	{

		/* Process the result to get the value in volts */
		if (adcConvDone == true)
		{
			/* Process the result to get the value in volts */
			/* adcValue = ((float) adcRawValue / adcMax) * (ADC_VREFH - ADC_VREFL); */
			adcValue = (( adcRawValue * 5000) / adcMax) ;// (ADC_VREFH - ADC_VREFL);
			/* Clear conversion done interrupt flag */
			adcConvDone = false;
			/* Trigger PDB timer */
			PDB_DRV_SoftTriggerCmd(PDB_INSTANCE);
		}
		else
		{
			/* do nothing*/
		}



		/* activate / deactivate fan  */
		if(adcValue>2500)
		{
			/* Turn the cooler ON */
			PINS_DRV_ClearPins(COOLER_PORT, (1 << COOLER_PIN));
		}
		else
		{
			/* Turn the cooler OFF */
			PINS_DRV_SetPins(COOLER_PORT, (1 << COOLER_PIN));
		}


	}

	return exit_code;
}




/* Functions implementation section*/

/* Clock system init function*/
void v_SystemInit(void)
{
    /* Function call Init section ---------------------------------------------*/
	/* Initialize clock module */  
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

}


/*DRV init function */
void v_DrvInit(void)
{

	/* Initialize pins
	 *    -    See PinSettings component for more info
	 */
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    /* Initialize LPIT instance 0
     *  -   Reset and enable peripheral
     */
    LPIT_DRV_Init(INST_LPIT_CONFIG_1, &lpit1_InitConfig);
    /* Initialize LPIT channel 0 and configure it as a periodic counter
     * which is used to generate an interrupt every second.
     */
    /* Initialize LPIT channel 0 and configure it as a periodic counter
        * which is used to generate an interrupt every second.
        */
    LPIT_DRV_InitChannel(INST_LPIT_CONFIG_1, LPIT_CHANNEL, &lpit1_ChnConfig0);



    /* Install v_LpitISR as LPIT interrupt handler */
    INT_SYS_InstallHandler(LPIT_Channel_IRQn, &v_LpitISR, NULL);

    /* Start LPIT0 channel 0 counter */
    LPIT_DRV_StartTimerChannels(INST_LPIT_CONFIG_1, (1 << LPIT_CHANNEL));



    PINS_DRV_SetPins(STP_DRV_EN_PORT, (1 << STP_DRV_EN_PIN));
    PINS_DRV_ClearPins(STP_DRV_EN_PORT, (1 << STP_DRV_EN_PIN));

    PINS_DRV_SetPins(STP_DRV_DIR_PORT, (1 << STP_DRV_DIR_PIN));
    PINS_DRV_ClearPins(STP_DRV_DIR_PORT, (1 << STP_DRV_DIR_PIN));

}


/*Button init function*/
void v_ButtonInit(void)
{
    /* Setup Button pins */
    PINS_DRV_SetPinsDirection(BTN_GPIO, ~((1 << BTN1_PIN)|(1 << BTN2_PIN)));

    /* Setup button pins interrupt */
    PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_RISING_EDGE);
    PINS_DRV_SetPinIntSel(BTN_PORT, BTN2_PIN, PORT_INT_RISING_EDGE);

    /* Install buttons ISR */
    INT_SYS_InstallHandler(BTN_PORT_IRQn, &v_ButtonISR, NULL);

    /* Enable buttons interrupt */
    INT_SYS_EnableIRQ(BTN_PORT_IRQn);


}



/*ADC init function*/
void v_AdcInit(void)
{

	/* Configure and calibrate the ADC converter
	 *  -   See ADC component for the configuration details
	 */
	DEV_ASSERT(ADC_0_ChnConfig0.channel == ADC_CHN);

	ADC_DRV_ConfigConverter(ADC_INSTANCE, &ADC_0_ConvConfig0);
	ADC_DRV_AutoCalibration(ADC_INSTANCE);
	ADC_DRV_ConfigChan(ADC_INSTANCE, 0UL, &ADC_0_ChnConfig0);


    INT_SYS_InstallHandler(ADC0_IRQn, &v_AdcISR, (isr_t*) 0);

     /* Calculate the value needed for PDB instance
     * to generate an interrupt at a specified timeout.
     * If the value can not be reached, stop the application flow
     */
	if (!calculateIntValue(&pdb_1_timerConfig0, PDLY_TIMEOUT, &delayValue))
    {
        /* Stop the application flow */
	   while(1);
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
    PDB_DRV_SetAdcPreTriggerDelayValue(PDB_INSTANCE, 0UL, 0UL, (uint32_t) delayValue);
    PDB_DRV_LoadValuesCmd(PDB_INSTANCE);
    PDB_DRV_SoftTriggerCmd(PDB_INSTANCE);

    /* Enable ADC 1 interrupt */
    INT_SYS_EnableIRQ(ADC0_IRQn);


}



/* @brief: ADC Interrupt Service Routine.
 *        Read the conversion result, store it
 *        into a variable and set a specified flag.
 */
void v_AdcISR(void)
{
    /* Get channel result from ADC channel */
    ADC_DRV_GetChanResult(ADC_INSTANCE, 0U, (uint16_t *)&adcRawValue);
    /* Set ADC conversion complete flag */
    adcConvDone = true;
}

/*!
 * @brief: LPIT interrupt handler.
 *         When an interrupt occurs clear channel flag and toggle DRV step
 */
void v_LpitISR(void)
{
	if (LPIT_DRV_GetInterruptFlagTimerChannels(INST_LPIT_CONFIG_1,(1 << LPIT_CHANNEL)))
	{
		/* Clear LPIT channel flag */
		LPIT_DRV_ClearInterruptFlagTimerChannels(INST_LPIT_CONFIG_1, (1 << LPIT_CHANNEL));

        /* Toggle driver step*/
		PINS_DRV_TogglePins(STP_DRV_STEP_PORT, (uint32_t) (1u << STP_DRV_STEP_PIN));
	}
}



/** * Button interrupt handler D */
void v_ButtonISR(void)
{
    /* Check if one of the buttons (SW3/SW2) was pressed */
    uint32_t buttonsPressed = PINS_DRV_GetPortIntFlag(BTN_PORT) &
                                           ((1 << BTN1_PIN) | (1 << BTN2_PIN));


    if(buttonsPressed != 0)
    {
        /* Set LED PWM duty value according to the button pressed */
        switch (buttonsPressed)
        {
            case (1 << BTN1_PIN):

            	/* start / stop stepper motor */
            	PINS_DRV_TogglePins(STP_DRV_EN_PORT, (uint32_t) (1u << STP_DRV_EN_PIN));

                /* Clear interrupt flag */
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
                break;


            case (1 << BTN2_PIN):
                //ledRequested = LED_BRIGHTNESS_DECREASE_REQUESTED;

            	/* set direction for stepper motor */
            	PINS_DRV_TogglePins(STP_DRV_DIR_PORT, (uint32_t) (1u << STP_DRV_DIR_PIN));

                /* Clear interrupt flag */
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN2_PIN);
                break;

            default:
                //do nothing
                break;
        }
    }
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
bool calculateIntValue(const pdb_timer_config_t *pdbConfig, uint32_t uSec, uint16_t * intVal)
{
    /* Local variables used to store different parameters
     * such as frequency and prescalers
     */
    uint32_t    intVal_l            = 0;
    uint8_t     pdbPrescaler        = (1 << pdbConfig->clkPreDiv);
    uint8_t     pdbPrescalerMult    = 0;
    uint32_t    pdbFrequency;

    bool resultValid = false;

    /* Get the Prescaler Multiplier from the configuration structure */
    switch (pdbConfig->clkPreMultFactor)
    {
        case PDB_CLK_PREMULT_FACT_AS_1:
            pdbPrescalerMult    =   1U;
            break;
        case PDB_CLK_PREMULT_FACT_AS_10:
            pdbPrescalerMult    =   10U;
            break;
        case PDB_CLK_PREMULT_FACT_AS_20:
            pdbPrescalerMult    =   20U;
            break;
        case PDB_CLK_PREMULT_FACT_AS_40:
            pdbPrescalerMult    =   40U;
            break;
        default:
            /* Defaulting the multiplier to 1 to avoid dividing by 0*/
            pdbPrescalerMult    =   1U;
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
    if((intVal_l == 0) || (intVal_l >= (1 << 16)))
    {
        resultValid = false;
        (*intVal) = 0U;
    }
    else
    {
        resultValid = true;
        (*intVal) = (uint16_t)intVal_l;
    }

    return resultValid;
}


