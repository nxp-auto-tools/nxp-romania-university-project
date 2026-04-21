/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "clock_utils.h"

/* Functions ---------------------------------------------------------------------- */

/* @brief: Initializes and updates the system clock configuration using predefined settings.
 *
 * @return: no return value
 * @return type: void
 */
void v_clockInit(void) {
    CLOCK_SYS_Init(g_clockManConfigsArr,
                   CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr,
                   CLOCK_MANAGER_CALLBACK_CNT);

    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
}
