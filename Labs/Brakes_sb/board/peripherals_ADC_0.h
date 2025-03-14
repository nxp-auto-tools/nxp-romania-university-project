/***********************************************************************************************************************
 * This file was generated by the S32 Config Tools. Any manual edits made to this file
 * will be overwritten if the respective S32 Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef ADC_0_H
#define ADC_0_H

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
#include "adc_driver.h"

/*******************************************************************************
 * Definitions 
 ******************************************************************************/

/*Device instance number */
#define INST_ADC_0  (1U)

/*******************************************************************************
 * Global variables 
 ******************************************************************************/

/* User configurations */

/* Converter configuration 0 */
extern const adc_converter_config_t ADC_0_ConvConfig0;

/* Hw Compare configuration 0 */
extern const adc_compare_config_t ADC_0_HwCompConfig0;

/* Hw Average configuration 0 */
extern const adc_average_config_t ADC_0_HwAvgConfig0;

/* Channel configuration 0 */
extern adc_chan_config_t ADC_0_ChnConfig0;



#endif /* ADC_0_H */
