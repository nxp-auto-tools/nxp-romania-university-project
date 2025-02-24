/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdk_project_config.h"
#include "helper_functions.h"
#include <string.h>
#include "S32K144.h"

// pin port configuration
#define LED0_PORT PTD
#define LED0_PIN  15 			// RGB_RED
#define LED1_PORT PTD
#define LED1_PIN  16 			// RGB_GREEN
#define LED2_PORT PTD
#define LED2_PIN  0 			// RGB_BLUE
#define LED_STRIP_PORT PTD  	// WS2811 pin
#define LED_STRIP_PIN 6

#define BUTTON_RIGHT_TURN_PORT PTE
#define BUTTON_RIGHT_TURN_PIN  15
#define BUTTON_LEFT_TURN_PORT PTE
#define BUTTON_LEFT_TURN_PIN 16
#define BUTTON_BRAKE_TURN_PORT PTE
#define BUTTON_BRAKE_TURN_PIN 14
#define BUTTON_DAYLIGHT_PORT PTD
#define BUTTON_DAYLIGHT_PIN 7
#define BUTTON_HIGH_BEAM_PORT PTE
#define BUTTON_HIGH_BEAM_PIN 1
#define BUTTON_GREEN_PORT PTE
#define BUTTON_GREEN_PIN 13

#define NUM_LEDS 10  // Number of WS2811 LEDs

#define T0H 0 // High time for "0" bit (in cycles, adjust based on clock
#define T0L 2 // Low time for "0" bit
#define T1H 2 // High time for "1" bit
#define T1L 0 // Low time for "1" bit
#define RESET_TIME 100 // Latch time after sending data

// LED index mapping

#define LED_RD_R  0  // Right direction, rear
#define LED_BRK_R1  1  // Brake, rear LED 1
#define LED_BRK_R2  2  // Brake, rear LED 2
#define LED_LD_R  3  // Left direction, rear
#define LED_RD_F  4  // Right direction, front
#define LED_HB_F1  5  // High beam, front LED 1
#define LED_DL_F1  6  // Daylight, front LED 1
#define LED_DL_F2  7  // Daylight, front LED 2
#define LED_HB_F2  8  // High beam, front LED 2
#define LED_LD_F  9  // Left direction, front

// LED color definitions (GRB format)
const uint8_t COLOR_OFF[3] = { 0, 0, 0 };
const uint8_t COLOR_LOW_WHITE[3] = { 125, 125, 125 };
const uint8_t COLOR_WHITE[3] = { 255, 255, 255 };
const uint8_t COLOR_RED[3] = { 0, 255, 0 };
const uint8_t COLOR_YELLOW[3] = { 255, 255, 0 };

// State variables
uint8_t daylightOn = 0;
uint8_t highBeamOn = 0;
uint8_t rightTurnOn = 0;
uint8_t leftTurnOn = 0;
uint8_t ledData[NUM_LEDS][3] = { 0 }; // Array for LED colors, each LED has 3 bytes (G, R, B)
uint8_t tempLedData[NUM_LEDS][3] = { 0 }; // Array for LED colors, each LED has 3 bytes (G, R, B)

/**
 * Sends a single bit to the LED strip using the WS2811 timing protocol.
 *
 * This function sets the data line high and low for durations that correspond
 * to either a '0' bit or a '1' bit, based on the WS2811 protocol specifications.
 *
 * @param bit The bit value to send: 1 to send a '1' bit, 0 to send a '0' bit.
 *
 * Timing for the high and low periods are determined by T1H, T1L (for '1') and
 * T0H, T0L (for '0'). These constants should be defined such that they match the
 * timing requirements for the WS2811 LED driver IC.
 */
void sendBit(uint8_t bit) {
	if (bit) {
		// Send '1' bit: high for T1H, then low for T1L
		LED_STRIP_PORT->PSOR = (1 << LED_STRIP_PIN); 	// Set pin high
		delayCycles(T1H);								// High for T1H
		LED_STRIP_PORT->PCOR = (1 << LED_STRIP_PIN); 	// Set pin low
		delayCycles(T1L);								// Low for T1L
	} else {
		// Send '0' bit: high for T0H, then low for T0L
		LED_STRIP_PORT->PSOR = (1 << LED_STRIP_PIN); 	// Set pin high
		delayCycles(T0H);                           	// High for T0H
		LED_STRIP_PORT->PCOR = (1 << LED_STRIP_PIN); 	// Set pin low
		delayCycles(T0L);                           	// Low for T0L
	}
}

/**
 * Sends a single byte to the LED strip using WS2811 timing.
 *
 * This function iterates through each bit of the byte, starting from
 * the most significant bit (MSB) and moving to the least significant
 * bit (LSB). Each bit is sent by calling the sendBit() function.
 *
 * @param byte The 8-bit value to send.
 */
void sendByte(uint8_t byte) {
	for (int8_t i = 7; i >= 0; i--) {
		sendBit((byte >> i) && 0x01);
	}
}

/**
 * Sends data to a series of WS2811 LEDs.
 *
 * This function sends the color data (in GRB order) for each LED by iterating through
 * the array of LED color values. For each LED, it sends the Green, Red, and Blue components
 * in sequence. Once all data is sent, it waits for a reset time to ensure the LED driver latches
 * the new values.
 *
 * @param data A 2D array containing the GRB color data for each LED.
 * @param numLEDs The number of LEDs in the array.
 */
void sendLEDData(uint8_t data[][3], size_t numLEDs) {
	// For each LED, send RGB values in order
	for (size_t i = 0; i < numLEDs; i++) {
		sendByte(data[i][0]); // Green
		sendByte(data[i][1]); // Red
		sendByte(data[i][2]); // Blue
	}
	// After sending data, delay to latch
	delayCycles(RESET_TIME);
}

void delay(volatile int cycles) {
	/* Delay function - do nothing for a number of cycles */
	while (cycles--)
		;
}

/**
 * Updates the color of a specific LED in the ledData array.
 *
 * This function sets the color values (G, R, B) for a given LED
 * in the ledData array. The provided color array must have exactly
 * three elements (green, red, blue) corresponding to the WS2811 format.
 *
 * @param ledIndex The index of the LED to update in the ledData array.
 * @param color A pointer to an array of three uint8_t values representing the color (G, R, B).
 */
void updateLEDColor(uint8_t (*ledArray)[3], uint8_t ledIndex, const uint8_t color[3]) {
	ledArray[ledIndex][0] = color[0]; // G
	ledArray[ledIndex][1] = color[1]; // R
	ledArray[ledIndex][2] = color[2]; // B
}

/**
 * Sets all LEDs to full brightness (white color).
 *
 * This function updates the ledData array so that each LED is assigned
 * the maximum intensity for all three color channels (G, R, B), effectively
 * turning them all on as white. After updating the array, it calls sendLEDData
 * to transmit the new values to the LEDs.
 */
void updateLEDsAllOn(void) {
	// Set all LEDs to white
	for (int i = 0; i < NUM_LEDS; i++) {
		updateLEDColor(ledData, i, COLOR_WHITE);
	}
	sendLEDData(ledData, NUM_LEDS);
}

/**
 * Turns off all LEDs by setting their colors to zero.
 *
 * This function sets each LED in the ledData array to a color value of (0, 0, 0),
 * effectively turning them all off. It then sends the updated data to the LEDs.
 */
void updateLEDsAllOff(void) {
	// Turn off all LEDs
	for (int i = 0; i < NUM_LEDS; i++) {
		updateLEDColor(ledData, i, COLOR_OFF);
	}
	sendLEDData(ledData, NUM_LEDS);
}

void checkLEDs(void) {
	for (int i = 0; i < NUM_LEDS; i++) {
		updateLEDColor(ledData,i, COLOR_WHITE);
		sendLEDData(ledData, NUM_LEDS);
		delay(2000000);
	}

	for (int i = 0; i < NUM_LEDS; i++) {
		updateLEDColor(ledData,i, COLOR_OFF);
		sendLEDData(ledData, NUM_LEDS);
		delay(2000000);
	}

}

int main(void) {
	status_t error;
	/* Configure clocks for PORT */
	error = CLOCK_DRV_Init(&clockMan1_InitConfig0);
	DEV_ASSERT(error == STATUS_SUCCESS);
	/* Set pins as GPIO */
	error = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
	DEV_ASSERT(error == STATUS_SUCCESS);

	updateLEDsAllOff();
	//  check all LEDs are working
	checkLEDs();

	delay(1520000);

	// Infinite loop to continuously check button states and update LEDs
	for (;;) {

		// Toggle daylight mode on button press
		if (!(BUTTON_DAYLIGHT_PORT->PDIR & (1 << BUTTON_DAYLIGHT_PIN))) {
			daylightOn ^= 1;
		}

		// If daylight is off, high beam should also be off
		// TODO

		// Toggle high beam mode if daylight is on and button is pressed
		// TODO

		// Update LED colors based on daylight status
		// TODO
	

		// Update high beam LEDs based on high beam status
		// TODO


		// Check brake button state and update brake LEDs accordingly
		// TODO


		// Check right turn signal button state and update LEDs accordingly
		// TODO

		// Check left turn signal button state and update LEDs accordingly
		// TODO

		// Flash left and/or right turn signal LEDs if any turn signal is active
		// TODO

		// If no turn signal is active, send the current LED data as is
		// TODO

		// default action
		sendLEDData(ledData, NUM_LEDS);

		delay(1520000);
	}

}
