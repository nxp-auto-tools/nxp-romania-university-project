/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*!
 * @file lpuart_irq.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, Function not defined with external linkage.
 * The functions are not defined static because they are referenced in .s startup files.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in driver c file, so it must remain global.
 */

#include "lpuart_irq.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (LPUART_INSTANCE_COUNT > 0U)
/* Implementation of LPUART0 handler named in startup code. */
void LPUART0_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(0);
}
#endif

#if (LPUART_INSTANCE_COUNT > 1U)
/* Implementation of LPUART1 handler named in startup code. */
void LPUART1_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(1);
}
#endif

#if (LPUART_INSTANCE_COUNT > 2U)
/* Implementation of LPUART2 handler named in startup code. */
void LPUART2_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(2);
}
#endif

#if (LPUART_INSTANCE_COUNT > 3U)
/* Implementation of LPUART3 handler named in startup code. */
void LPUART3_IrqHandler(void)
{
    LPUART_DRV_IRQHandler(3);
}
#endif

/* Array storing references to LPUART irq handlers */
isr_t g_lpuartIsr[LPUART_INSTANCE_COUNT] =
{
#if (LPUART_INSTANCE_COUNT > 0U)
    LPUART0_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 1U)
    LPUART1_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 2U)
    LPUART2_IrqHandler,
#endif
#if (LPUART_INSTANCE_COUNT > 3U)
    LPUART3_IrqHandler,
#endif
};

/*******************************************************************************
 * EOF
 ******************************************************************************/
