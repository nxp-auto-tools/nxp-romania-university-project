/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "adc_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define ADC_CHN                (ADC_INPUTCHAN_EXT12)
#define ADC_RESOLUTION_BITS    (12U)
#define ADC_MAX_VALUE          (1U << ADC_RESOLUTION_BITS)

/* Functions ---------------------------------------------------------------------- */
/* @brief: Initializes the ADC (Analog to Digital Converter) module.
 *
 * @param adcInstance: pointer to ADC (Analog to Digital Converter) instance configuration
 * @param type: adc_instanceConfig_t*
 *
 * @return: no return value
 * @return type: void
 */
void v_adcInit(adc_instanceConfig_t* adcInstance) {
    /* configure and calibrate the ADC (Analog to Digital Converter) converter */
    /* see ADC (Analog to Digital Converter) component for the configuration details */
    DEV_ASSERT(adcInstance->channelConfig->channel == ADC_CHN);

    ADC_DRV_ConfigConverter(adcInstance->instance, adcInstance->converterConfig);
    ADC_DRV_AutoCalibration(adcInstance->instance);
    ADC_DRV_ConfigChan(adcInstance->instance, adcInstance->chanIndex, adcInstance->channelConfig);
}

/* @brief: Scales the latest ADC (Analog to Digital Converter) conversion result proportionally to a user-defined maximum value.
 *
 * @param scaleMax: the maximum value to scale the ADC (Analog to Digital Converter) result to
 * @param type: uint16_t
 * @param ui16AdcValue: pointer to ADC (Analog to Digital Converter) value to be scaled
 * @param type: uint16_t*
 *
 * @return: the scaled ADC (Analog to Digital Converter) value
 * @return type: uint16_t
 */
void v_scaleAdcValue(uint16_t scaleMax, uint16_t* ui16AdcValue) {
//	 TODO 1: Convert ADC value to percentage
//	 Formula: (current_value * desired_max) / actual_max
//	 Hint: ADC_MAX_VALUE = 4096 (12 bits)

    (*ui16AdcValue) = -1/*Complete here*/;
}
