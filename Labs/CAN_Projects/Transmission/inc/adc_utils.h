/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ADC_UTILS_H_
#define ADC_UTILS_H_

/* Include files ------------------------------------------------------------------ */
#include "sdk_project_config.h"
#include "interrupt_manager.h"

/* Macro -------------------------------------------------------------------------- */

typedef struct{
    uint8_t instance;
    uint8_t chanIndex;
    adc_chan_config_t* channelConfig;
    const adc_converter_config_t* converterConfig;
    uint16_t adcValue;
    bool adcConvDone;
    IRQn_Type irqNumber;
    isr_t isrHandler;
}adc_instanceConfig_t;

/* Function prototypes ------------------------------------------------------------ */

/* @brief: Initializes the ADC (Analog to Digital Converter) module. */
void v_adcInit(adc_instanceConfig_t* adcInstance);

/* @brief: Scales the latest ADC (Analog to Digital Converter) conversion result proportionally to a user-defined maximum value. */
void v_scaleAdcValue(uint16_t scaleMax, uint16_t* ui16AdcValue);

#endif /* ADC_UTILS_H_ */
