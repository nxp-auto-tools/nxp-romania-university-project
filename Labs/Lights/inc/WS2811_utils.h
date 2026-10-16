/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef WS2811_UTILS_H
#define WS2811_UTILS_H

#include "helper_functions.h"
#include "sdk_project_config.h"
#include "lights_utils.h"

/* WS2811 timing parameters expressed in CPU cycles */
#define T0H 0U /* High time for "0" bit (in cycles) */
#define T0L 2U /* Low time for "0" bit */
#define T1H 2U /* High time for "1" bit */
#define T1L 0U /* Low time for "1" bit */

#define RESET_TIME 100U

/* LED colors (positions are G,R,B) */
extern const uint8_t ui8ColorOff[3];
extern const uint8_t ui8ColorLowWhite[3];
extern const uint8_t ui8ColorWhite[3];
extern const uint8_t ui8ColorYellow[3];
extern const uint8_t ui8ColorGreen[3];
extern const uint8_t ui8ColorRed[3];
extern const uint8_t ui8ColorNXPYellow[3];
extern const uint8_t ui8ColorNXPBlue[3];
extern const uint8_t ui8ColorNXPGreen[3];

/* Function prototypes ------------------------------------------------------------ */

/* @brief Transmit a single byte MSB-first using WS2811 bit timings. */
void v_sendBit(uint8_t ui8Bit);

/* @brief Transmit a single byte MSB-first using WS2811 bit timings. */
void v_sendByte(uint8_t ui8Byte);

/* @brief Send an entire LED buffer to the strip in GRB */
void v_sendLEDData(uint8_t ui8Data[][3], size_t ui8NumLEDs);

/* @brief Update a single LED's color in a GRB buffer. */
void v_updateLEDColor(uint8_t (*ui8LedArray)[3], uint8_t ui8LedIndex,
        const uint8_t ui8Color[3]);

/* @brief Set all LEDs to white and transmit immediately. */
void v_updateLEDsAllOn(uint8_t (*ui8LedData)[3]);

/* @brief Clear (turn off) all LEDs and transmit immediately. */
void v_updateLEDsAllOff(uint8_t (*ui8LedData)[3]);

#endif
