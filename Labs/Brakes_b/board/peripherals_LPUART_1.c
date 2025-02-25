/***********************************************************************************************************************
 * This file was generated by the S32 Configuration Tools. Any manual edits made to this file
 * will be overwritten if the respective S32 Configuration Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v7.0
processor: S32K144
package_id: S32K144_LQFP100
mcu_data: s32sdk_s32k1xx_rtm_401
processor_version: 0.0.0
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 31debf36-e6ca-4958-b4fc-99be0fa08669
  called_from_default_init: true
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Included files
 ******************************************************************************/
#include "peripherals_LPUART_1.h"

/*******************************************************************************
 * LPUART_1 initialization code
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LPUART_1'
- type: 'lpuart_config'
- mode: 'general'
- custom_name_enabled: 'true'
- type_id: 'lpuart'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'LPUART_1'
- config_sets:
  - lpuart_driver:
    - lpuart_state_name: 'lpUartState1'
    - lpuart_configuration:
      - 0:
        - name: 'lpUartInitConfig1'
        - readonly: 'false'
        - transferType: 'LPUART_USING_INTERRUPTS'
        - baudRate: '115200'
        - parityMode: 'LPUART_PARITY_DISABLED'
        - stopBitCount: 'LPUART_ONE_STOP_BIT'
        - bitCountPerChar: 'LPUART_8_BITS_PER_CHAR'
        - rxDMAChannel: '0'
        - txDMAChannel: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External variable could be made static.
 * The external variables will be used in other source files in application code.
 *
 */
lpuart_state_t lpUartState1;

lpuart_user_config_t lpUartInitConfig1 = {
  .transferType = LPUART_USING_INTERRUPTS,
  .baudRate = 115200UL,
  .parityMode = LPUART_PARITY_DISABLED,
  .stopBitCount = LPUART_ONE_STOP_BIT,
  .bitCountPerChar = LPUART_8_BITS_PER_CHAR,
  .rxDMAChannel = 0UL,
  .txDMAChannel = 0UL
};


