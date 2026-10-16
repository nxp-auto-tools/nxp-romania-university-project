/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*!
 * @file edma_irq.h
 */

#include "edma_hw_access.h"

/*! @brief DMA channel interrupt handler, implemented in driver c file. */
void EDMA_DRV_IRQHandler(uint8_t virtualChannel);
#ifdef FEATURE_DMA_HAS_ERROR_IRQ
/*! @brief DMA error interrupt handler, implemented in driver c file. */
void EDMA_DRV_ErrorIRQHandler(uint8_t virtualChannel);
#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
