/*
* Copyright (c) 2015 - 2016 , Freescale Semiconductor, Inc.
 * Copyright 2016-2025 NXP
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
#ifndef BRAKES_UTILS_H
#define BRAKES_UTILS_H

#include "helper_functions.h"
#include "sdk_project_config.h"

/* Number of WS2811 LEDs */
#define NUM_LEDS 6U
/* Port connected to LED strip --> used to send the color encoding */
#define LED_STRIP_PORT PTE
#define LED_STRIP_PIN  14U

/* Latch time after sending data */
#define DELAY_TIME 0x000FFFU
#define BLINK_TOGGLE_LIMIT 250U
/* Self test delay time between 2 leds */
#define SELF_TEST_DELAY_TIME 5000000U

/* Led Strip Setup --------------------------------------------------------------- */
/* 6-LED bar --> 8 LED states (all off, 1 on, 2 on, ..., all on, emergency brakes) */
#define LED_STATES_NUMBER 7U
/* 24 bits per LED color coding (currently not used) */
#define LED_BITS   24U
/* Used to set the 8-bits of a specific color */
#define NUMBER_OF_BITS 8

/* Function prototypes ------------------------------------------------------------ */

/* @brief Displays a sequence of NXP brand colors on the LED strip. */
void v_checkLEDsNXPColors(uint8_t (*ui8LedData)[3]);
/* @brief Runs a self-test by lighting LEDs in a sequence. */
void v_checkLEDsSequential(uint8_t (*ui8LedData)[3]);
/* @brief Updates the LED strip with active colors based on the current state. */
void v_displayActiveLEDs(uint8_t (*ui8LedData)[3], uint8_t ui8LedsStates);
/* @brief Toggles emergency LED blinking when brake is fully pressed. */
void v_blinkEmergencyLEDs(uint8_t (*ui8LedData)[3], uint8_t ui8LedsON);

#endif
