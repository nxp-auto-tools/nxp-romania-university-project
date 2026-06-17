/*
 * Copyright (c) 2015 - 2016 , Freescale Semiconductor, Inc.
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and that
 * you agree to comply with and are bound by, such license terms. If you do not
 * agree to be bound by the applicable license terms, then you may not retain,
 * install, activate or otherwise use the software. The production use license
 * in Section 2.3 is expressly granted for this software.
 */

#include "WS2811_utils.h"

/* Constant colors (GRB) ---------------------------------------------------- */
const uint8_t ui8ColorOff[3]       = {   0,   0,   0 };
const uint8_t ui8ColorLowWhite[3]  = {  10,  10,  10 };
const uint8_t ui8ColorWhite[3]	   = { 255, 255, 255 };
const uint8_t ui8ColorYellow[3]    = { 255, 255,   0 };
const uint8_t ui8ColorGreen[3]     = { 255,   0,   0 };
const uint8_t ui8ColorRed[3]       = {   0, 255,   0 };
const uint8_t ui8ColorNXPYellow[3] = { 180, 255,   0 };
const uint8_t ui8ColorNXPBlue[3]   = { 100,  10, 255 };
const uint8_t ui8ColorNXPGreen[3]  = { 255,  50,   0 };

/* Functions ------------------------------------------------------------------ */

/* @brief: Sends a single bit to the LED strip using WS2811 timing protocol.
 *
 * @details: This function transmits a single bit to the LED strip by
 * manipulating the data line according to the WS2811 timing specifications.
 * If `ui8Bit` is 1, the function sends a '1' bit using the timing constants
 * `T1H` (high duration) and `T1L` (low duration). If `ui8Bit` is 0,
 * it sends a '0' bit using `T0H` and `T0L`. The data line is controlled via
 * `LED_STRIP_PORT` and `LED_STRIP_PIN`, using bitwise operations to set or
 * clear the pin.
 *
 * @param ui8Bit: The bit value to send (1 for '1' bit, 0 for '0' bit).
 *
 * @return: no return value
 * @return type: void
 */
void v_sendBit(uint8_t ui8Bit) {
    if (ui8Bit) {
        /* Send '1' bit: high for T1H, then low for T1L */
        LED_STRIP_PORT->PSOR = ( 1 << LED_STRIP_PIN );
        v_delayCycles(T1H);
        LED_STRIP_PORT->PCOR = ( 1 << LED_STRIP_PIN );
        v_delayCycles(T1L);
    } else {
        /* Send '0' bit: high for T0H, then low for T0L */
        LED_STRIP_PORT->PSOR = ( 1 << LED_STRIP_PIN );
        v_delayCycles(T0H);
        LED_STRIP_PORT->PCOR = ( 1 << LED_STRIP_PIN );
        v_delayCycles(T0L);
    }
}

/* @brief: Sends a single byte to the LED strip using WS2811 timing protocol,
 * bit by bit.
 *
 * @details: This function transmits an 8-bit value (`ui8Byte`) to the LED
 * strip by iterating through each bit from the most significant bit (MSB) to
 * the least significant bit (LSB). For each bit, it calls `v_sendBit()` to
 * send the appropriate signal according to the WS2811 timing protocol.
 * This ensures that the LED strip receives the correct data sequence for
 * color control.
 *
 * @param ui8Byte: The 8-bit value to send to the LED strip.
 *
 * @return: no return value
 * @return type: void
 */
void v_sendByte(uint8_t ui8Byte) {
    for (int8_t i = 7; i >= 0; i--) {
        v_sendBit((ui8Byte >> i) && 0x01);
    }
}

/**
 * @brief: Sends data to a series of WS2811 LEDs.
 *
 * @details: This function transmits color data to a chain of WS2811-controlled
 * LEDs. For each LED, it sends the color components in GRB order (Green, Red,
 * Blue) by calling `v_sendByte()` for each component. The input is a array
 * where each row contains the GRB values for one LED. After all data is sent,
 * the function waits for a reset period (`RESET_TIME`) to allow the LED driver
 * to latch the new values.
 *
 * @param ui8Data: A array containing the GRB color data for each LED.
 * @param ui8NumLEDs: The number of LEDs in the array.
 *
 * @return: no return value
 * @return type: void
 */
void v_sendLEDData(uint8_t ui8Data[][3], size_t ui8NumLEDs) {
    /* For each LED, send RGB values in order */
    for (size_t i = 0; i < ui8NumLEDs; i++) {
        v_sendByte(ui8Data[i][0]);
        v_sendByte(ui8Data[i][1]);
        v_sendByte(ui8Data[i][2]);
    }
    /* After sending ui8Data, delay to latch */
    v_delayCycles(RESET_TIME);
}

/* @brief: Updates the color of a specific LED in the ledData array using
 * WS2811 format (G, R, B).
 *
 * @param ledArray: pointer to the array containing LED color data.
 * @param type: uint8_t (*)[3]
 *
 * @param ui8LedIndex: index of the LED to be updated.
 * @param type: uint8_t
 *
 * @param ui8Color: array containing the new color values in GRB format
 * (3 bytes: Green, Red, Blue)
 * @param type: const uint8_t[3]
 *
 * @return: no return value
 * @return type: void
 */
void v_updateLEDColor(uint8_t (*ui8LedArray)[3], uint8_t ui8LedIndex,
        const uint8_t ui8Color[3]) {

    /**
     * TODO: Copy color values from ui8Color array to ui8LedData at the specified LED index
     * Hint: You need to copy 3 bytes (Green, Red, Blue) from source to destination,
     * LED data format is [LED_number][color_component]
    */

}

/* @brief: Sets all LEDs in the ledData array to full brightness (white color)
 * and sends the updated data.
 *
 * @param ui8LedData: pointer to the array containing LED color data.
 * @param type: uint8_t (*)[3]
 *
 * @return: no return value
 * @return type: void
 */
void v_updateLEDsAllOn(uint8_t (*ui8LedData)[3]) {
    // Set all LEDs to white
    for (int i = 0; i < NUM_LEDS; i++) {
        v_updateLEDColor(ui8LedData, i, ui8ColorWhite);
    }
    v_sendLEDData(ui8LedData, NUM_LEDS);
}

/* @brief: Turns off all LEDs by setting their color values to zero and sends
 * the updated data.
 *
 * @param ui8LedData: pointer to the array containing LED color data.
 * @param type: uint8_t (*)[3]
 *
 * @return: no return value
 * @return type: void
 */
void v_updateLEDsAllOff(uint8_t (*ui8LedData)[3]) {
    /* Turn off all LEDs */
    for (int i = 0; i < NUM_LEDS; i++) {
        v_updateLEDColor(ui8LedData, i, ui8ColorOff);
    }
    v_sendLEDData(ui8LedData, NUM_LEDS);
}
