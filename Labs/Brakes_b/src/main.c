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

#include "sdk_project_config.h"
#include "helper_functions.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define welcomeStr "This is an ADC example, it will show you the value converted"\
                   "\r\nfrom ADC1 Input 11 (or ADC0 Input 12 for EVB )\r\n"

#define headerStr  "ADC result: "

/* ADC macro definitions */
#define ADC_INSTANCE    0UL
#define ADC_CHN         ADC_INPUTCHAN_EXT12
#define ADC_VREFH       5.0f
#define ADC_VREFL       0.0f

/* 8-LED bar --> 9 LED states (all off, 1 on, 2 on...) */
#define LED_STATES_NUMBER   9U  
/* total number of LEDs on the strip */
#define LED_NUMBER          8U
/* 24 bits per LED color coding (currently not used) */
#define LED_BITS            24U 
/* Used to set the 8-bits of a specific color */
#define NUMBER_OF_BITS      8U  

/* Port connected to LED strip --> used to send the color encoding */
#define LED_STRIP_PORT PTD
#define LED_STRIP_PIN  1 << 0U

/* Delays for the LED strip control */
#define DELAY_50        1 // Will be used to generate a "1" logic - both high and low
#define DELAY_25_high   0 // Will be used to generate a "0" logic
#define DELAY_25_low    2 // Will be used to generate a "0" logic

/* TODO: Determine the ADC min and max levels to be used to "normalize" the sensor output */
#define ADC_MIN 0
#define ADC_MAX 0
#define ADC_STEP ((ADC_MAX - ADC_MIN)/8.0)

/* Counter to determine how many LEDs must be on depending on the ADC value */
uint8_t leds_on = 0;

typedef struct RGB
{
	uint8_t g;
	uint8_t r;
	uint8_t b;
} RGB_t;

/* TODO: Configure LED colors (positions are G,R,B) */
RGB_t leds_values[LED_NUMBER] = {
		{0, 0, 0},      /* LED color: GREEN */
		{0, 0, 0},      /* LED color: GREEN */
		{0, 0, 0},      /* LED color: YELLOW */
		{0, 0, 0},      /* LED color: YELLOW */
		{0, 0, 0},      /* LED color: ORANGE */
		{0, 0, 0},      /* LED color: ORANGE */
		{0, 0, 0},      /* LED color: RED */
		{0, 0, 0},      /* LED color: RED */
};

/* This function controls the logic to generate the value "1" for LED control */
void oneLogic()
{
	LED_STRIP_PORT->PSOR = GPIO_PSOR_PTSO(LED_STRIP_PIN);
	delayCycles(DELAY_50);
	LED_STRIP_PORT->PCOR = GPIO_PCOR_PTCO(LED_STRIP_PIN);
	delayCycles(DELAY_50);
}

/* This function controls the logic to generate the value "0" for LED control */
void zeroLogic()
{
	LED_STRIP_PORT->PSOR = GPIO_PSOR_PTSO(LED_STRIP_PIN);
	delayCycles(DELAY_25_high);
	LED_STRIP_PORT->PCOR = GPIO_PCOR_PTCO(LED_STRIP_PIN);
	delayCycles(DELAY_25_low);
}

/* 
 * This function iterates through all the bits that control one LED color to generate 
 * the 8-bit control signal.
 */
void set_color(uint8_t color)
{
    /* Implement coding logic to set the desired color. Call oneLogic() and zeroLogic(). */
}


/* 
 * Function used to send the ADC data through UART (COM port). 
 * It will help us decide the min and max values of our interval (for the pressure sensor).
 */
void print(const char *sourceStr)
{
    uint32_t bytesRemaining;

    /* Send data via LPUART */
    LPUART_DRV_SendData(INST_LPUART_1, (uint8_t *) sourceStr, strlen(sourceStr));
    /* Wait for transmission to be successful */
    while (LPUART_DRV_GetTransmitStatus(INST_LPUART_1, &bytesRemaining) != STATUS_SUCCESS)
    {
    }
}

/* 
 * This function calls set_color in order to construct the 24-bit signal
 * to control the color generation for one LED.
 */
void setColor(uint8_t green, uint8_t red, uint8_t blue) {
	set_color(green);
	set_color(red);
	set_color(blue);
}

/* 
 * This function iterates through "leds_on" and sets each LED color for 
 * the LEDs that must be on, and turns the LEDs off as needed.
 */
void print_LEDs() 
{

    /* TODO: Implement the control logic to generate a state for the LED strip
     * Note: all LED states must be handled here
     */

}


/*!
 \brief The main function for the project.
 \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
 */
int main(void)
{
	/* Enable the floating point unit */
	enableFPU();

	/* Write your local variable definition here */
	/* Variables in which we store data from ADC */
	uint16_t adcRawValue;
	uint16_t adcMax;
	float adcValue;

	/* Initialize and configure clocks
	 *  -   see clock manager component for details
	 */
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

	/* Initialize pins
	 *  -   See PinSettings component for more info
	 */
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
	LED_STRIP_PORT->PCOR = GPIO_PCOR_PTCO(LED_STRIP_PIN);

	/* Get ADC max value from the resolution (the ADC is configured with 12 bits resolution) */
	adcMax = (uint16_t) (1 << 12);

	if (LPUART_DRV_Init(INST_LPUART_1, &lpUartState1, &lpUartInitConfig1) != STATUS_SUCCESS)
		__asm("bkpt #255");

	/* Configure and calibrate the ADC converter
	 *  -   See ADC component for the configuration details
	 */
	ADC_DRV_ConfigConverter(ADC_INSTANCE, &ADC_0_ConvConfig0);
	ADC_DRV_AutoCalibration(ADC_INSTANCE);

	/* Send a welcome message via LPUART */
	print(welcomeStr);


	/************/
	/* Buffer used to store processed data for serial communication */
	char msg[255] = {0};


	/* Infinite loop:
	 *  - Trigger a new conversion
	 *  - Wait and get the result
	 *  - Make the value more user friendly
	 *  - Control the LED strip
	 */

	while (1)
	{
		/* Configure ADC channel and software trigger a conversion */
		ADC_DRV_ConfigChan(ADC_INSTANCE, 0U, &ADC_0_ChnConfig0);
		/* Wait for the conversion to be done */
		ADC_DRV_WaitConvDone(ADC_INSTANCE);
		/* Store the channel result into a local variable */
		ADC_DRV_GetChanResult(ADC_INSTANCE, 0U, &adcRawValue);

		/* Process the result to get the value in volts */
		adcValue = ((float) adcRawValue / adcMax) * (ADC_VREFH - ADC_VREFL);

		floatToStr(&adcValue, msg, 5);
		/* Send the result to the user via LPUART */
		print(headerStr);
		print(msg);
		print(" V\r\n");

		/* Do nothing for a number of cycles */
		delayCycles(0x0FFFFF);

        /* TODO: Compute the number of LEDs to be turned on depending on the adc value */
		// leds_on = 0;
        /* print_LEDs uses the global leds_on to turn on or off the needed LEDs */
		print_LEDs();
	}
}

