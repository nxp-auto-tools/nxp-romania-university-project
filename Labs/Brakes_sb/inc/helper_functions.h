/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_HELPER_FUNCTIONS_H_
#define SOURCES_HELPER_FUNCTIONS_H_

#include "device_registers.h"
#include <stdint.h>

/***********************************
 * @brief: Convert a float to null terminated char array
 * @param[in] srcValue:  pointer to the source float value
 * @param[out] destStr:   pointer to the destination string
 * @param[in] maxLen:    maximum lenght of the string
 ***********************************/
void v_floatToStr(const float *srcValue, char *destStr, uint8_t maxLen);

/***********************************
 * @brief: Wait for a number of cycles
 * @param[in] nbOfCycles is number of cycles to be waited for
 ********************************/
void v_delayCycles(uint32_t nbOfCycles);

/***********************************
 * @brief Enables the on-device FPU
 * @param None
 * @return None
 ***********************************/
void v_enableFPU(void);

#endif /* SOURCES_HELPER_FUNCTIONS_H_ */
