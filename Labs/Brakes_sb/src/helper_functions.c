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

#include "helper_functions.h"
#include <stdint.h>
#include <stdbool.h>

/***********************************
 * @brief: Convert a float to null terminated char array
 * @param[in] srcValue:  pointer to the source float value
 * @param[out] destStr:   pointer to the destination string
 * @param[in] maxLen:    maximum lenght of the string
 ***********************************/
void v_floatToStr (const float *srcValue, char *destStr, uint8_t maxLen)
{
  /* Step 1: Initialize variables for conversion */
  uint8_t i, lessThanOne = 0;
  float tempVal = (*srcValue);
  uint8_t currentVal;

  /* Step 2: Handle negative values by prepending '-' */
  if (tempVal < 0)
    {
      tempVal *= -1;
      *destStr = '-';
      destStr++;
    }

  /* Step 3: Loop through each digit position */
  for (i = 0; i < maxLen; i++)
    {
      /* Step 4: Extract integer part of float */
      currentVal = (uint8_t) (tempVal);

      /* Step 5: Convert digit to ASCII and store */
      *destStr = currentVal + 48;
      destStr++;

      /* Step 6: Subtract the integer part to isolate decimal */
      tempVal -= currentVal;

      /* Step 7: Insert decimal point once */
      if ((tempVal < 1) && (lessThanOne == 0))
	 	{
	  		*destStr = '.';
	  		destStr++;
	  		lessThanOne = 1;
		}

	  /* Step 8: Shift decimal for next digit */
      tempVal *= 10;
    }

  /* Step 9: Null-terminate the string */
  *destStr = 0;
}

/***********************************
 * @brief: Wait for a number of cycles
 * @param[in] nbOfCycles is number of cycles to be waited for
 ***********************************/
void v_delayCycles (uint32_t nbOfCycles)
{
  /* Step 1: Busy-wait loop for delay */
  volatile uint32_t i = nbOfCycles;
  while (i--) /* Step 2: Empty loop body to consume cycles */
    ;
}

/***********************************
 * @brief Enables the on-device FPU
 * @param None
 * @return None
 ***********************************/
void v_enableFPU(void)
{
    /* Step 1: Set CPACR bits to enable FPU access */
	/* Enable FPU set both CPACR[CP11] and CPACR[CP10] to Full Access - 0b11 */
	S32_SCB->CPACR |= (S32_SCB_CPACR_CP10_MASK | S32_SCB_CPACR_CP11_MASK);
}
