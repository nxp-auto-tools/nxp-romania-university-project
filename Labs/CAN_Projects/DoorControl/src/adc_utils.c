/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "adc_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define ADC_RESOLUTION    (12)
#define ADC_MAX_VALUE     (1<<ADC_RESOLUTION)

/* Global variables --------------------------------------------------------------- */

/* Functions ---------------------------------------------------------------------- */
/* @brief: Initializes the ADC (Analog to Digital Converter) module.
 *
 * @param adcInstance:      pointer to a structure containing configuration data for ADC instance
 * @param type:             adc_instanceConfig_t*
 *
 * @return: no return value
 * @return type: void
 */
void v_adcInit(adc_instanceConfig_t *adcInstance) {

    /* configure and calibrate the ADC (Analog to Digital Converter) converter */
    /* see ADC (Analog to Digital Converter) component for the configuration details */
    DEV_ASSERT(adcInstance->channelConfig->channel== adcInstance->inputChan);

    ADC_DRV_ConfigConverter(adcInstance->instance,
                            adcInstance->converterConfig);

    ADC_DRV_AutoCalibration(adcInstance->instance);

    ADC_DRV_ConfigChan(adcInstance->instance,
                       adcInstance->chanIndex,
                       adcInstance->channelConfig);
}

/* @brief: Scales the latest ADC (Analog to Digital Converter) conversion result proportionally to a user-defined maximum value.
 *
 * @param scaleMax:         the maximum value(in mV) to scale the ADC (Analog to Digital Converter) result to
 * @param type:             uint16_t
 * @param ui16AdcValue:     pointer to the adc variable(in mV) to be converted
 * @param type:             uint16_t*
 *
 * @return: the scaled ADC (Analog to Digital Converter) value
 * @return type: uint16_t
 */

void v_scaleAdcValue(uint16_t scaleMax, uint16_t *ui16AdcValue) {
    /* Process the result to get the value in mV */
    (*ui16AdcValue) = (((*ui16AdcValue) * scaleMax) / ADC_MAX_VALUE);
}
