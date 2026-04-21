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
#include "can_utils.h"

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
#define LED_STRIP_PIN  1 << 7U

/* Delays for the LED strip control */
#define DELAY_50        1 // Will be used to generate a "1" logic - both high and low
#define DELAY_25_high   0 // Will be used to generate a "0" logic
#define DELAY_25_low    2 // Will be used to generate a "0" logic

/* ADC min and max levels to be used to "normalize" the sensor output */
#define ADC_MIN 2.25
#define ADC_MAX 2.522

/* Smoothing Factor */
#define ALPHA 0.1

/* The message buffer used by FlexCAN to transmit data */
#define TX_MAILBOX 9
#define RX_MAILBOX 7

/* Counter to determine how many LEDs must be on depending on the ADC value */
uint8_t leds_on = 0;

uint8_t brakesPushed = 0;
float filteredValue = 2.53f;

typedef struct RGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RGB_t;

static flexcan_data_info_t txInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U
};

static flexcan_data_info_t rxInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U
};

/* Buffer used to store received CAN messages */
flexcan_msgbuff_t rx_msg;

/* LED colors (positions are G,R,B) */
RGB_t leds_values[LED_NUMBER] = {
		{0, 255,   0},      /* LED color: GREEN */
		{0, 255,   0},      /* LED color: GREEN */
		{255, 255, 0},      /* LED color: YELLOW */
		{255, 255, 0},      /* LED color: YELLOW */
		{255, 128, 0},      /* LED color: ORANGE */
		{255, 128, 0},      /* LED color: ORANGE */
		{255,   0, 0},      /* LED color: RED */
		{255,   0, 0},      /* LED color: RED */
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
	for (int i = NUMBER_OF_BITS - 1; i >= 0; i--)
	{
		if (color & (1 << i))
		{
			oneLogic();
		} else
		{
			zeroLogic();
		}
	}
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
void print_LEDs() {

	for (uint8_t i = 0; i < leds_on; i++)
	{
		setColor(leds_values[i].g, leds_values[i].r, leds_values[i].b);
	}
	for (uint8_t i = 0; i < LED_STATES_NUMBER - leds_on; i++)
	{
		setColor(0, 0, 0);
	}
	delayCycles(640);

}


uint8_t calculateLedsOn(){
	return (uint8_t)((ADC_MAX - filteredValue) / (ADC_MAX - ADC_MIN) * LED_NUMBER);
}

/*
 * This is a callback function that is called upon when a message via FlexCAN
 * is received, it checks if the brakes button is pushed and updates the global brakesPushed variable
 */
void flexcanCallback(uint8_t instance,
                     flexcan_event_type_t eventType,
                     uint32_t buffIdx,
                     flexcan_state_t *flexcanState) {
    if(eventType == FLEXCAN_EVENT_RX_COMPLETE && buffIdx == RX_MAILBOX){
        /* 
         * TODO 6: Implement the FlexCAN callback logic for RX.
         * Hint: Check if the eventType is FLEXCAN_EVENT_RX_COMPLETE.
         * If it is, extract data from 'rx_msg' and update flags.
         * You also need to re-arm the RX buffer using FLEXCAN_DRV_Receive().
         */
    }

}

/*!
 \brief The main function for the project.
 \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
 */
int main(void)

{
	/* Variables for FlexCAN transmission */
	uint8_t txData[1] = { 0 };
	uint8_t previousTxData[1] = { 0 };

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

	/* Initialize flexcan
	 *  -   See flexcan component for more info
	 */
    /*
     * TODO 0: Initialize the FlexCAN module.
     * Hint: Use the function FLEXCAN_DRV_Init(uint8_t instance, flexcan_state_t *state, const flexcan_user_config_t *config).
     * You should use 'INST_FLEXCAN_CONFIG_1', '&flexcanState0', and '&flexcanInitConfig0'.
     */

    /* 
     * TODO 3: Configure the TX message buffer for sending brake data.
     * Hint: Use the function FLEXCAN_DRV_ConfigTxMb(instance, mb_idx, &tx_info, msg_id).
     */

    /* 
     * TODO 1: Configure the RX message buffer for receiving data.
     * Hint: Use the function FLEXCAN_DRV_ConfigRxMb(instance, mb_idx, &rx_info, msg_id).
     */

    /*  Installs a callback function for the FlexCAN IRQ handler  */
    FLEXCAN_DRV_InstallEventCallback(INST_FLEXCAN_CONFIG_1, &flexcanCallback, NULL);

    /* 
     * TODO 2: Start receiving data on the configured mailbox.
     * Hint: Use the function FLEXCAN_DRV_Receive(instance, mb_idx, &rx_msg).
     */


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

	/* Buffer used to store processed data for serial communication */
	char msg[255] = {0};

	/* Values used for the pressure sensor calibration */
	uint8_t last_leds_on = 255;

	/* Infinite loop:
	 *  - Trigger a new conversion
	 *  - Wait and get the result
	 *  - Make the value more user friendly
	 *  - Control the LED strip
	 *  - Send the CAN Message
	 */

	while (1)
	{
		if(!brakesPushed){
			/* Configure ADC channel and software trigger a conversion */
			ADC_DRV_ConfigChan(ADC_INSTANCE, 0U, &ADC_0_ChnConfig0);
			/* Wait for the conversion to be done */
			ADC_DRV_WaitConvDone(ADC_INSTANCE);
			/* Store the channel result into a local variable */
			ADC_DRV_GetChanResult(ADC_INSTANCE, 0U, &adcRawValue);

			/* Process the result to get the value in volts */
			adcValue = ((float) adcRawValue / adcMax) * (ADC_VREFH - ADC_VREFL);

			/* Smoothing the adcValue */
			filteredValue += ALPHA * (adcValue - filteredValue);

			/*  Clamp the adcValue */
			if (filteredValue > ADC_MAX) filteredValue = ADC_MAX;
			if (filteredValue < ADC_MIN) filteredValue = ADC_MIN;


			/* Send the result to the user via LPUART */
			floatToStr(&adcValue, msg, 5);
			print(headerStr);
					print(msg);
					print(" V\r\n");

			/* Calculate the leds intensity (0-8) based on ADC sensor */
			leds_on = calculateLedsOn();
		}else {
			leds_on = LED_NUMBER;
		}
		/* Do nothing for a number of cycles */
		delayCycles(0x0FF);

		/* Update local LED strip only when the state changes */
		if (leds_on != last_leds_on) {
			last_leds_on = leds_on;
			print_LEDs();
		}

		/* 
		 * TODO 4: Populate the transmission data for the HMI project.
		 * Hint: The HMI project expects the number of LEDs currently on in 'txData[0]'.
		 */

		/* 
		* TODO 5: Send the CAN message with the brake data.
		* Hint: Use the function CAN_SendData(uint8_t mailbox, uint32_t msg_id, uint8_t txData[], flexcan_data_info_t *txInfo), make sure the can TX message buffer is not busy before sending.
		*/
	}
}



