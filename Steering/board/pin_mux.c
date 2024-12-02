/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v16.0
processor: S32K144
package_id: S32K144_LQFP100
mcu_data: s32sdk_s32k1xx_rtm_401
processor_version: 0.0.0
pin_labels:
- {pin_num: '22', pin_signal: PTD15, label: LED_RED_PWM, identifier: LED_RED_PWMLED_RED_PWM;LED_RED_PWM}
- {pin_num: '21', pin_signal: PTD16, label: LED_GREEN_PWM, identifier: LED_GREEN_PWM}
- {pin_num: '81', pin_signal: PTC6, label: UART_RX}
- {pin_num: '80', pin_signal: PTC7, label: UART_TX}
- {pin_num: '4', pin_signal: PTD0, label: LED_BLUE_PWM, identifier: LED_BLUE_PWM}
- {pin_num: '46', pin_signal: PTC14, label: ADC_POT}
- {pin_num: '94', pin_signal: PTE0, label: ECU_HMI, identifier: ECU_HMI}
- {pin_num: '50', pin_signal: PTC12, label: BTN2_SW3_PIN, identifier: BTN2_SW3_PIN}
- {pin_num: '49', pin_signal: PTC13, label: BTN3_SW2_PIN, identifier: BTN3_SW2_PIN}
- {pin_num: '48', pin_signal: PTB2, label: Servo_pwm}
- {pin_num: '83', pin_signal: PTA15, label: ADC_POT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External variable could be made static.
 * The external variables will be used in other source files in application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 */

#include "pin_mux.h"

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0}
- pin_list:
  - {pin_num: '83', peripheral: ADC1, signal: 'se, 12', pin_signal: PTA15}
  - {pin_num: '48', peripheral: FTM1, signal: 'ch, 0', pin_signal: PTB2, direction: OUTPUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* Generate array of configured pin structures */
pin_settings_config_t g_pin_mux_InitConfigArr0[NUM_OF_CONFIGURED_PINS0] = {
    {
        .base            = PORTA,
        .pinPortIdx      = 15U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_PIN_DISABLED,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT2,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
};
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
