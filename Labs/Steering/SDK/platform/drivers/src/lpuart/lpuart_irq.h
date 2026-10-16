/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef LPUART_IRQ_H__
#define LPUART_IRQ_H__

#include "device_registers.h"
#include "interrupt_manager.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LPUART_DRV_IRQHandler(uint32_t instance);

/*******************************************************************************
 *  Default interrupt handlers signatures
 ******************************************************************************/

#if (LPUART_INSTANCE_COUNT > 0U)
/*! @brief LPUART0 interrupt handler. */
void LPUART0_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 1U)
/*! @brief LPUART1 interrupt handler. */
void LPUART1_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 2U)
/*! @brief LPUART2 interrupt handler. */
void LPUART2_IrqHandler(void);
#endif

#if (LPUART_INSTANCE_COUNT > 3U)
/*! @brief LPUART3 interrupt handler. */
void LPUART3_IrqHandler(void);
#endif

/*! Array storing references to LPUART irq handlers */
extern isr_t g_lpuartIsr[LPUART_INSTANCE_COUNT];

#endif /* LPUART_IRQ_H__ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
