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
#include "brakes_utils.h"
#include "WS2811_utils.h"

const uint8_t* ui8NXPColorRefs[NUM_LEDS] = {
    ui8ColorNXPYellow,
    ui8ColorNXPYellow,
    ui8ColorNXPBlue,
    ui8ColorNXPBlue,
    ui8ColorNXPGreen,
    ui8ColorNXPGreen
};
const uint8_t* ui8LedValues[NUM_LEDS] = {
    ui8ColorGreen,
    ui8ColorGreen,
    ui8ColorYellow,
    ui8ColorYellow,
    ui8ColorRed,
    ui8ColorRed
};

/* Functions ------------------------------------------------------------------ */

/**
 * @brief Displays a sequence of NXP brand colors on the LED strip.
 *
 * This function sets the LED colors to yellow, blue, and green,
 * representing NXP's brand identity. The colors are stored in
 * ui8NXPColors[] and applied directly to the LED buffer.
 *
 * Implements requirement SW_BR_008
 *
 * @param ui8LedData Array holding GRB values for each LED.
 * @return void
 */
void v_checkLEDsNXPColors(uint8_t (*ui8LedData)[3]) {
    for (uint8_t ui8LedIdx  = 0; ui8LedIdx < NUM_LEDS; ui8LedIdx++) {
        v_updateLEDColor(ui8LedData, ui8LedIdx, ui8NXPColorRefs[ui8LedIdx]);
    }

    v_sendLEDData(ui8LedData, NUM_LEDS);
}

/**
 * @brief Runs a self-test by lighting LEDs in a sequence.
 *
 * Each LED is turned on with a color from ui8NXPColors[], then turned off.
 * The sequence runs forward and then backward across all LEDs.
 *
 * Implements requirement SYS_BR_005
 *
 * @param ui8LedData
 *   Pointer to a 2D array holding RGB values for each LED.
 *
 * @return
 *   None.
 */
void v_checkLEDsSequential(uint8_t (*ui8LedData)[3]) {
    /* Step up: 0 to NUM_LEDS */
    for (uint8_t ui8LedIdx = 0; ui8LedIdx < NUM_LEDS; ui8LedIdx++) {
        v_updateLEDColor(ui8LedData, ui8LedIdx, ui8NXPColorRefs[ui8LedIdx]);
        v_sendLEDData(ui8LedData, NUM_LEDS);
        v_delayCycles(SELF_TEST_DELAY_TIME / 4);
        v_updateLEDColor(ui8LedData, ui8LedIdx, ui8ColorOff);
    }

    /* Step down: NUM_LEDS to 0 */
    for (uint8_t ui8LedIdx = NUM_LEDS; ui8LedIdx > 0; ui8LedIdx--) {
        v_updateLEDColor(ui8LedData, ui8LedIdx-1,ui8NXPColorRefs[ui8LedIdx-1]);
        v_sendLEDData(ui8LedData, NUM_LEDS);
        v_delayCycles(SELF_TEST_DELAY_TIME / 4);
        v_updateLEDColor(ui8LedData, ui8LedIdx-1, ui8ColorOff);
    }
}

/**
 * @brief Updates the LED strip with active colors based on the current state.
 *
 * This function lights up the first `ui8LedsStates` LEDs using the values
 * from `ui8LedValues[]`. Remaining LEDs are left unchanged.
 *
 * @param ui8LedData    Array holding GRB values for each LED.
 * @param ui8LedsStates Number of LEDs to activate.
 * @return void
 */
void v_displayActiveLEDs(uint8_t (*ui8LedData)[3], uint8_t ui8LedsStates) {

    /**
     * TODO: Light up the first 'ui8LedsStates' number of LEDs
     * Variables: ui8LedsStates = how many LEDs should be active
     * ui8LedValues[] = array containing the colors for each LED position
     * Hint: You need to check each LED position and decide if it
     * should be colored (v_updateLEDColor()) or not
    */


    v_sendLEDData(ui8LedData, NUM_LEDS);
}

/**
 * @brief Blink all LEDs red at a fixed rate when in emergency state.
 *
 * Implements requirement SW_BR_009
 *
 * @param ledData  2D array [numLEDs][3] buffer to update and send.
 * @param numLEDs  Number of LEDs in the buffer.
 * @return void
 */
void v_blinkEmergencyLEDs(uint8_t (*ui8LedData)[3], uint8_t ui8NumLEDs)
{
    static bool     bBlinkToggle   = false; /* toggles LED state */
    static uint16_t ui16BlinkCounter  = 0;  /* counts cycles */
    static bool     bPreviousToggle = false;/* stores last toggle state */

    ui16BlinkCounter++;
    if (ui16BlinkCounter >= BLINK_TOGGLE_LIMIT) {
        bPreviousToggle = bBlinkToggle;
        bBlinkToggle = !bBlinkToggle;
        ui16BlinkCounter = 0;
    }

    if (bPreviousToggle != bBlinkToggle) {
        const uint8_t *ui8Color = bBlinkToggle ? ui8ColorRed : ui8ColorOff;
        for (uint8_t ui8LedIdx = 0; ui8LedIdx < ui8NumLEDs; ui8LedIdx++) {
            v_updateLEDColor(ui8LedData, ui8LedIdx, ui8Color);
        }
        v_sendLEDData(ui8LedData, ui8NumLEDs - 1);
    }
}
