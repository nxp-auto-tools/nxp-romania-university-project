/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#define PCC_CLOCK	PCC_PORTD_CLOCK
#define LED0_PORT PTD
#define LED0_PIN  15
#define LED1_PORT PTD
#define LED1_PIN  16
#define LED2_PORT PTD
#define LED2_PIN  0

#define BTN_GPIO        PTC
#define BTN1_PIN        13U
#define BTN2_PIN        12U
#define BTN_PORT        PORTC
#define BTN_PORT_IRQn   PORTC_IRQn

#include "sdk_project_config.h"
#include "interrupt_manager.h"

/** * Button interrupt handler - used to toggle outputs of RED/GREEN */
void buttonISR(void)
{
    /* Check if one of the buttons (SW3/SW2) was pressed */


    if()
    {
        /* Set LED PWM duty value according to the button pressed */
        switch ()
        {
            case ():
                /* Clear interrupt flag */
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
                break;
            case ():
                /* Clear interrupt flag */
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN2_PIN);
                break;
            default:
                PINS_DRV_ClearPortIntFlagCmd(BTN_PORT);
                break;
        }
    }
}


int main(void)
{
	/* Configure clocks for PORT */
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

	/* Set pins as GPIO */


	/* Set Output value LED0 & LED1 */



	/* Setup button pins interrupt */



	/* Install buttons ISR */



	/* Enable buttons interrupt */
	INT_SYS_EnableIRQ(BTN_PORT_IRQn);


	while(1)
	{

	}
}
