/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "helper_functions.h"
#include <stdint.h>
#include <stdbool.h>

/***********************************
 * @brief: Convert a float to null terminated char array
 * @param srcValue:  pointer to the source float value
 * @param destStr:   pointer to the destination string
 * @param maxLen:    maximum lenght of the string
 ***********************************/
void floatToStr(const float *srcValue, char *destStr, uint8_t maxLen)
{
	uint8_t i, lessThanOne = 0;
	float tempVal = (*srcValue);
	uint8_t currentVal;

    if (tempVal < 0)
    {
	    tempVal *= -1;
        *destStr = '-';
        destStr++;
    }

    for (i = 0; i < maxLen; i++)
    {
        currentVal = (uint8_t) (tempVal);
        *destStr = currentVal + 48;
        destStr++;
        tempVal -= currentVal;

        if ((tempVal < 1) && (lessThanOne == 0))
	    {
	        *destStr = '.';
	        destStr++;
	        lessThanOne = 1;
	    }

        tempVal *= 10;
    }

    *destStr = 0;
}

/***********************************
 * @brief: Wait for a number of cycles
 * @param nbOfCycles is number of cycles to be waited for
 ***********************************/
void delayCycles (uint32_t nbOfCycles)
{
    volatile uint32_t i = nbOfCycles;
    while (i--);
}

/* Method that enables on-device FPU
 * param: 	None
 * return:	None
 */

void enableFPU(void)
{
	/* Enable FPU set both CPACR[CP11] and CPACR[CP10] to Full Access - 0b11 */
	S32_SCB->CPACR |= (S32_SCB_CPACR_CP10_MASK | S32_SCB_CPACR_CP11_MASK);
}
