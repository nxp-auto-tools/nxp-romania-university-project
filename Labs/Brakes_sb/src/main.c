/*
 * Copyright 2020, 2024-2025 NXP
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
#include "WS2811_utils.h"
#include "brakes_utils.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ADC Setup --------------------------------------------------------------- */
/* Analog to Digital macro definitions */
#define ADC_INSTANCE    1UL
#define ADC_CHN         ADC_INPUTCHAN_EXT12
#define ADC_VREFH_mV    5000U
#define ADC_VREFL_mV    0U

/* ADC min and max levels to be used to "normalize" the sensor output */
#define ADC_MIN 0.4f
#define ADC_MAX 2.5f
#define ADC_STEP_mV (((ADC_MAX - ADC_MIN) * 1000) / NUM_LEDS)
#define TOUCH_SENSITIVITY 0.9f

/* Function Header --------------------------------------------------------- */
/* @brief Initialize & Configure floating point, clocks, pins and ADC */
void v_init(void);
/* @brief Converts brake sensor voltage to a corresponding LED state count. */
uint8_t ui8_getLEDsStates(float adcVoltage);

uint8_t ui8LedsStates = 0;
/* LED colors (positions are G,R,B) */
uint8_t ui8LedData[NUM_LEDS][3] = {0};

/**
 * @brief: The main function for the project. The startup initialization
 * sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void) {
    /*Initialize & Configure floating point, clocks, pins and ADC*/
    v_init();

    /* Self-tests */
    /* blink one led in series, with NXP color */
    v_checkLEDsSequential(ui8LedData);
    /* blink all LEDs in NXP color */
    v_checkLEDsNXPColors(ui8LedData);
    v_delayCycles(SELF_TEST_DELAY_TIME * 3U);
    /* Variables for ADC */
    uint16_t ui16AdcRawValue;
    /* Get ADC max value from the resolution */
    /* Implements requirement SYS_BR_002 */
    uint16_t ui16AdcMax = (1 << 12);
    float fAdcValue;

    /**
     *  @brief: Infinite Loop
     *  - Trigger a new conversion
     *  - Wait and get the result
     *  - Make the value more user friendly
     *  - Control the LED strip
    */
    while (1) {
        ADC_DRV_ConfigChan(ADC_INSTANCE, 0U, &ADC_0_ChnConfig0);
        ADC_DRV_WaitConvDone(ADC_INSTANCE);
        ADC_DRV_GetChanResult(ADC_INSTANCE, 0U, &ui16AdcRawValue);

        /**
         * TODO: Convert raw ADC value to voltage in millivolts
         * Variables:
         * ui16AdcRawValue = raw number from ADC (0 to ui16AdcMax)
         * ADC_VREFH_mV & ADC_VREFL_mV = maximum voltage reference, ADC_VREFL_mV = minimum voltage reference
         * ui16AdcMax = highest possible ADC reading
         * Hint: This is a proportion - raw value relates to the voltage range, 
         * like the max reading relates to the total range
         */
        fAdcValue = 0.0f;

        ui8LedsStates = ui8_getLEDsStates(fAdcValue);

        if (ui8LedsStates > NUM_LEDS) {
           /* Emergency blink across the whole led strip */
            v_blinkEmergencyLEDs(ui8LedData, ui8LedsStates);
        } else {
            v_displayActiveLEDs(ui8LedData, ui8LedsStates);
            v_updateLEDsAllOff(ui8LedData);
        }
        v_delayCycles(DELAY_TIME);
    }
}

/* Support Functions ------------------------------------------------------ */
/**
 * @brief: Initialize the floating point,
 * clocks - see ClockManager component,
 * pins   - see PinSettings component,
 * ADC    - see ADC component for configurations
 */
void v_init() {
    /* Enable the floating point unit */
    v_enableFPU();

    /* Initialize and configure clocks
     *  -   see clock manager component for details
     */
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    /* Initialize pins
     *  -   See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    LED_STRIP_PORT->PCOR = GPIO_PCOR_PTCO(LED_STRIP_PIN);

    /* Configure and calibrate the ADC converter
     *  -   See ADC component for the configuration details
     */
    ADC_DRV_ConfigConverter(ADC_INSTANCE, &ADC_0_ConvConfig0);
    ADC_DRV_AutoCalibration(ADC_INSTANCE);
}

/**
 * @brief Converts brake sensor voltage to a corresponding LED state count.
 *
 * This function maps the current analog sensor voltage input `fAdcVoltage`
 * from the brake sensor to a discrete number of LED states to be activated
 * on the LED bar, based on the total possible states.
 *
 * The LED states represent the following:
 * - 0: All LEDs off (no braking)
 * - 1 - 6: Progressive braking intensity
 * - 7: Emergency braking (activates blinking red pattern)
 *
 * The result is clamped to a maximum of `LED_STATES_NUMBER`, which
 * defines the full range of brake intensity visualization.
 * For a more accurate brake sensitivity reading, `TOUCH_SENSITIVITY` is used.
 *
 * Implements requirement SYS_BR_004
 *
 * @param fAdcVoltage Analog voltage from brake sensor (in mV).
 * @return Number of LED states to activate (0 to LED_STATES_NUMBER).
 */
uint8_t ui8_getLEDsStates(float fAdcVoltage)
{
    /**
     * TODO: Calculate how many LEDs should be active based on brake pressure
     * Hint: Higher voltage means less braking, fewer LEDs should be on
     */
}
