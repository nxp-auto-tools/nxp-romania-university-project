/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#define EVB

#ifdef EVB
	#define PCC_CLOCK	PCC_PORTD_CLOCK
	#define LED0_PORT PTD
	#define LED0_PIN  15
	#define LED1_PORT PTD
	#define LED1_PIN  16
#else
	#define PCC_CLOCK	PCC_PORTC_CLOCK
	#define LED0_PORT PTC
	#define LED0_PIN  0
	#define LED1_PORT PTC
	#define LED1_PIN  1
#endif

#include "sdk_project_config.h"

void delay(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--);
}

int main(void)
{
  status_t error;
  /* Configure clocks for PORT */
  error = CLOCK_DRV_Init(&clockMan1_InitConfig0);
  DEV_ASSERT(error == STATUS_SUCCESS);
  /* Set pins as GPIO */
  error = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
  DEV_ASSERT(error == STATUS_SUCCESS);

  /* Set Output value LED0 & LED1 */
  PINS_DRV_SetPins(LED0_PORT, 1 << LED0_PIN);
  PINS_DRV_ClearPins(LED1_PORT, 1 << LED1_PIN);

  for (;;)
  {
      /* Insert a small delay to make the blinking visible */
      delay(720000);

      /* Toggle output value LED0 & LED1 */
      PINS_DRV_TogglePins(LED0_PORT, 1 << LED0_PIN);
      PINS_DRV_TogglePins(LED1_PORT, 1 << LED1_PIN);
  }
}
