/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef SOURCES_HELPER_FUNCTIONS_H_
#define SOURCES_HELPER_FUNCTIONS_H_

#include "device_registers.h"
#include <stdint.h>

/***********************************
 * @brief: Convert a float to null terminated char array
 * @param srcValue:  pointer to the source float value
 * @param destStr:   pointer to the destination string
 * @param maxLen:    maximum lenght of the string
 ***********************************/
void floatToStr(const float *srcValue, char *destStr, uint8_t maxLen);

/***********************************
 * @brief: Wait for a number of cycles
 * @param nbOfCycles is number of cycles to be waited for
 ***********************************/
void delayCycles(uint32_t nbOfCycles);

/* Method that enables on-device FPU
 * param: 	None
 * return:	None
 */

void enableFPU(void);

#endif /* SOURCES_HELPER_FUNCTIONS_H_ */
