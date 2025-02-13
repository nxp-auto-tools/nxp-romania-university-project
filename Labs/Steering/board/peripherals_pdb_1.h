/*
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/***********************************************************************************************************************
 * This file was generated by the S32 Config Tools. Any manual edits made to this file
 * will be overwritten if the respective S32 Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef pdb_1_H
#define pdb_1_H

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * The global macro will be used in function call of the module.
 *
 */
/*******************************************************************************
 * Included files 
 ******************************************************************************/
#include "pdb_driver.h"

/*******************************************************************************
 * Global variables 
 ******************************************************************************/

/*! @brief Device instance number */
#define INST_PDB_0 0U

/*! @brief ADC pre-trigger configuration declarations */
extern const pdb_adc_pretrigger_config_t pdb_1_adcTrigConfig0;

/*! @brief PDB timer configuration declarations */
extern const pdb_timer_config_t pdb_1_timerConfig0;



#endif /* pdb_1_H */
