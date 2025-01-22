/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/* Include header files section*/
#include "sdk_project_config.h"
#include "interrupt_manager.h"

/* Data types*/
typedef unsigned char  t_uint8;
typedef   int8_t       t_sint8; // included in _stdint.h file
typedef unsigned short t_uint16;
typedef   signed short t_sint16;
typedef unsigned int   t_uint32;
typedef   signed int   t_sint32;


/*Define sections - Led Output Pins*/
#define PCC_CLOCK	        PCC_PORTD_CLOCK
#define LED_RGB_PORTD       PTD
#define LED_RGB_RED_PTD15    15
#define LED_RGB_GREEN_PTD16  16
#define LED_RGB_BLUE_PTD0     0


/* Define Buttons Button Output Ports/Pins/irq  */
#define BTN_GPIO        PTC
#define BTN_SW2        12U
#define BTN_SW3        13U
#define BTN_PORT        PORTC
#define BTN_PORT_IRQn   PORTC_IRQn

/*LPIT*/
/* LPIT channel used */
#define LPIT_CHANNEL        0UL
#define LPIT_Channel_IRQn   LPIT0_Ch0_IRQn

/* Function prototype section */

/*Description: System initialization function*/
void v_SystemInit(void);

/*Description: Button initialization function*/
void v_ButtonInit(void);

/*Description: Timer Initialization function*/
void v_TimerLpitInit(void);

/*Description: User function, delay for a number o cycles function */
void v_Delay(volatile int cycles);

/*Description: Button interrupt handler */
void v_ButtonISR(void);

/*Description: Timer interrupt function. Period: 1sec */
void v_TimerLpitISR(void);

/*Global variable declaration */
volatile t_uint8 ui8_testVar;

/* Main function*/
int main(void)
{
  /*Init Section*/
  /* System initialization */
  v_SystemInit();

  /* Init timer function call*/
  v_TimerLpitInit();

   /* Init button function call*/
   v_ButtonInit();

  /* Init global variable */
  ui8_testVar  = 200u;

  /* Endless loop*/
  while(1)
  {
      /* Toggle ON RED LED */
      PINS_DRV_TogglePins(LED_RGB_PORTD, 1 << LED_RGB_RED_PTD15);

      /* Insert a small delay to make the blinking visible */
	  v_Delay(5720000);

	  /*Increment for testing*/
	  ui8_testVar++;

  }
}


/*Description: System initialization function*/
void v_SystemInit(void)
{
	  status_t error;
	  /* Configure clocks for PORT */
	  error = CLOCK_DRV_Init(&clockMan1_InitConfig0);
	  DEV_ASSERT(error == STATUS_SUCCESS);
	  /* Set pins as GPIO */
	  error = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
	  DEV_ASSERT(error == STATUS_SUCCESS);
}



/*Description: Button initialization function*/
void v_ButtonInit(void)
{

	  /* Setup button pins interrupt */
	  PINS_DRV_SetPinIntSel(BTN_PORT, BTN_SW2, PORT_INT_RISING_EDGE);
	  PINS_DRV_SetPinIntSel(BTN_PORT, BTN_SW3, PORT_INT_RISING_EDGE);

	  /* Install buttons ISR */
	  INT_SYS_InstallHandler(BTN_PORT_IRQn, &v_ButtonISR, NULL);

	      /* Enable buttons interrupt */
	  INT_SYS_EnableIRQ(BTN_PORT_IRQn);

	  /* Turn OFF all leds */
	  PINS_DRV_SetPins(LED_RGB_PORTD, 1 << LED_RGB_BLUE_PTD0);
	  PINS_DRV_SetPins(LED_RGB_PORTD, 1 << LED_RGB_GREEN_PTD16);
	  PINS_DRV_SetPins(LED_RGB_PORTD, 1 << LED_RGB_RED_PTD15);

	  /* Turn ON only green led */
	  PINS_DRV_ClearPins(LED_RGB_PORTD, 1 << LED_RGB_GREEN_PTD16);

	  /* Insert a small delay to make the blinking visible */
	  v_Delay(5720000);

	  //Turn Off green led
	  PINS_DRV_SetPins(LED_RGB_PORTD, 1 << LED_RGB_GREEN_PTD16);
}

/*Description: Timer Initialization function*/
void v_TimerLpitInit(void)
{

	  /* Initialize LPIT instance 0
	    *  -   Reset and enable peripheral
	    */
	   LPIT_DRV_Init(INST_LPIT_CONFIG_1, &lpit1_InitConfig);
	   /* Initialize LPIT channel 0 and configure it as a periodic counter
	    * which is used to generate an interrupt every second.
	    */
	  LPIT_DRV_InitChannel(INST_LPIT_CONFIG_1, LPIT_CHANNEL, &lpit1_ChnConfig0);


	   /* Install LPIT_ISR as LPIT interrupt handler */
	   INT_SYS_InstallHandler(LPIT_Channel_IRQn, &v_TimerLpitISR, NULL);

	   /* Start LPIT0 channel 0 counter */
	   LPIT_DRV_StartTimerChannels(INST_LPIT_CONFIG_1, (1 << LPIT_CHANNEL));

	   /* Peripherals Initialization is complete, now the program will wait for
	    * LPIT interrupt.
	    */
}

/*Description: delay for a number o cycles function */
void v_Delay(volatile int cycles)
{
    /* Delay function - do nothing for a number of cycles */
    while(cycles--)
    {
    	//do nothing
    }
}

/*Description: Button interrupt handler */
void v_ButtonISR(void)
{

    uint32_t ui32_ButtonsPressed;

    /* Get interrupt flag status */
    ui32_ButtonsPressed = PINS_DRV_GetPortIntFlag(BTN_PORT) & ((1 << BTN_SW2) | (1 << BTN_SW3));

    /* Check if one of the buttons (SW3/SW2) was pressed */
    if(ui32_ButtonsPressed != 0u)
    {
        switch (ui32_ButtonsPressed)
        {
            case (1 << BTN_SW2):
				/* Turn OFF Blue led */
				PINS_DRV_SetPins(LED_RGB_PORTD, 1 << LED_RGB_BLUE_PTD0);

                //Clear interrupt flag
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN_SW2);
                break;

            case (1 << BTN_SW3):
				/*Turn ON blue led*/
				PINS_DRV_ClearPins(LED_RGB_PORTD, 1 << LED_RGB_BLUE_PTD0);

                /* Clear interrupt flag */
                PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN_SW3);
                break;

            default:
                //do nothing
                break;
        }
    }
}


/*Description: Timer interrupt function. Period: 1sec */
void v_TimerLpitISR(void)
{
	if (LPIT_DRV_GetInterruptFlagTimerChannels(INST_LPIT_CONFIG_1,(1 << LPIT_CHANNEL)))
	{
		/* Clear LPIT channel flag */
		LPIT_DRV_ClearInterruptFlagTimerChannels(INST_LPIT_CONFIG_1, (1 << LPIT_CHANNEL));

		/* Add my code/actions */
		/* Toggle ON Green LED */
		PINS_DRV_TogglePins(LED_RGB_PORTD, 1 << LED_RGB_GREEN_PTD16);
	}
}
