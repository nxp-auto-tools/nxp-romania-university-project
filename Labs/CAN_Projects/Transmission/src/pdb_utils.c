/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Include files ------------------------------------------------------------------ */
#include "pdb_utils.h"

/* Macro -------------------------------------------------------------------------- */
#define PDB_INIT_ZERO            (0U)
#define PDB_PREMULT_1            (1U)
#define PDB_PREMULT_10           (10U)
#define PDB_PREMULT_20           (20U)
#define PDB_PREMULT_40           (40U)
#define MHZ_CONVERSION_FACTOR    (1000000U)
#define MAX_16BIT_VALUE          (65536U)

/* Functions ---------------------------------------------------------------------- */
/* @brief: Initializes PDB to generate periodic interrupts for ADC (Analog to Digital Converter) triggering.
 *
 * @param ui16delayValue: calculated delay value for the PDB
 * @param type: uint16_t
 *
 * @return: no return value
 * @return type: void
 */
void v_pdbInit(pdb_instanceConfig_t* pdbInstance){
    uint16_t ui16delayValue = PDB_INIT_ZERO;

    /* calculate the delay value for the PDB timer interrupt */
    if (!b_calculateIntValue(pdbInstance->timerConfig, pdbInstance->pdlyTimeout_microS,&ui16delayValue)) {
        /* if calculation fails, stop the program */
        while (1);
    }

    /* set up the PDB (Programmable Delay Block) */
    PDB_DRV_Init(pdbInstance->instance, pdbInstance->timerConfig);
    /* enable the PDB module */
    PDB_DRV_Enable(pdbInstance->instance);
    /* configure ADC (Analog to Digital Converter) trigger settings */
    PDB_DRV_ConfigAdcPreTrigger(pdbInstance->instance, pdbInstance->chn, pdbInstance->adcTrigConfig);
    /* set the timer delay value */
    PDB_DRV_SetTimerModulusValue(pdbInstance->instance, (uint32_t) ui16delayValue);
    /* set the delay for ADC (Analog to Digital Converter) trigger */
    PDB_DRV_SetAdcPreTriggerDelayValue(pdbInstance->instance, pdbInstance->chn, pdbInstance->preChn, (uint32_t)ui16delayValue);
    /* load all configured values into the PDB */
    PDB_DRV_LoadValuesCmd(pdbInstance->instance);
    /* start the PDB using a software trigger */
    PDB_DRV_SoftTriggerCmd(pdbInstance->instance);
}

/* @brief: Calculate the values to be used by PDB to generate a interrupt at a specific timeout.
 *
 * @param pdbConfig: pointer to the PDB configuration struct
 * @param type:      const pdb_timer_config_t *
 * @param uSec:      interval for PDB interrupt in microseconds
 * @param type:      uint32_t
 * @param intVal:    pointer to the storage element where to set the calculated value
 * @param type:      uint16_t
 * @return:          returns true if the interrupt period can be achieved, false if not
 * @return type:     bool
 */
bool b_calculateIntValue(const pdb_timer_config_t *pdbConfig, uint32_t uSec, uint16_t *intVal) {
    uint32_t ui32Temp = PDB_INIT_ZERO;
    uint8_t ui8PdbPrescaler = (1U << pdbConfig->clkPreDiv);
    uint8_t ui8PdbPrescalerMult = PDB_INIT_ZERO;
    uint32_t ui32PdbFrequency;

    bool bResultValid = false;

    /* set multiplier based on configuration */
    switch (pdbConfig->clkPreMultFactor) {
    case PDB_CLK_PREMULT_FACT_AS_1:
        ui8PdbPrescalerMult = PDB_PREMULT_1;
        break;
    case PDB_CLK_PREMULT_FACT_AS_10:
        ui8PdbPrescalerMult = PDB_PREMULT_10;
        break;
    case PDB_CLK_PREMULT_FACT_AS_20:
        ui8PdbPrescalerMult = PDB_PREMULT_20;
        break;
    case PDB_CLK_PREMULT_FACT_AS_40:
        ui8PdbPrescalerMult = PDB_PREMULT_40;
        break;
    default:
        ui8PdbPrescalerMult = PDB_PREMULT_1;
        break;
    }

    /* get the system clock frequency in MHz */
    CLOCK_SYS_GetFreq(CORE_CLOCK, &ui32PdbFrequency);
    ui32PdbFrequency /= MHZ_CONVERSION_FACTOR;

    /* calculate the interrupt value based on time and clock settings */
    ui32Temp = (ui32PdbFrequency * uSec) / (ui8PdbPrescaler * ui8PdbPrescalerMult);

    /* check if the result fits in a 16-bit value */
    if ((ui32Temp == PDB_INIT_ZERO) || (ui32Temp >= MAX_16BIT_VALUE)) {
        bResultValid = false;
        (*intVal) = PDB_INIT_ZERO;
    } else {
        bResultValid = true;
        (*intVal) = (uint16_t)ui32Temp;
    }

    return bResultValid;
}
