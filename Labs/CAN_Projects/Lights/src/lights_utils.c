
#include "lights_utils.h"


// LED index mapping
#define LED_RD_R 0   // Right direction, rear
#define LED_BRK_R1 1 // Brake, rear LED 1
#define LED_BRK_R2 2 // Brake, rear LED 2
#define LED_LD_R 3   // Left direction, rear
#define LED_RD_F 4   // Right direction, front
#define LED_HB_F1 5  // High beam, front LED 1
#define LED_DL_F1 6  // Daylight, front LED 1
#define LED_DL_F2 7  // Daylight, front LED 2
#define LED_HB_F2 8  // High beam, front LED 2
#define LED_LD_F 9   // Left direction, front



/* @brief: Updates the daylight LEDs based on the current daylight status, setting them to white or turning them off.
 *
 * @details: If `bDaylightStatus` is active, the function sets the daylight LEDs to low white brightness.
 * Otherwise, it turns them off. It updates two specific LEDs: `LED_DL_F1` and `LED_DL_F2`.
 *
 * @return: no return value
 * @return type: void
 */
void v_updateDaylightLEDs(bool bDaylightStatus, uint8_t (*ui8LedData)[3])
{
    if (bDaylightStatus)
    {
        v_updateLEDColor(ui8LedData, LED_DL_F1, ui8ColorLowWhite);
        v_updateLEDColor(ui8LedData, LED_DL_F2, ui8ColorLowWhite);
    }
    else
    {
        v_updateLEDColor(ui8LedData, LED_DL_F1, ui8ColorOff);
        v_updateLEDColor(ui8LedData, LED_DL_F2, ui8ColorOff);
    }
}

/* @brief: Updates the high beam LEDs based on the current high beam status, setting them to white or turning them off.
 *
 * @details: If `ui8RighBeamStatus` is active, the function sets the high beam LEDs (`LED_HB_F1` and `LED_HB_F2`)
 * to white. Otherwise, it turns them off.
 *
 * @return: no return value
 * @return type: void
 */
void v_updateHighBeamLEDs(bool bHighBeamStatus, uint8_t (*ui8LedData)[3])
{
    if (bHighBeamStatus)
    {
        v_updateLEDColor(ui8LedData, LED_HB_F1, ui8ColorWhite);
        v_updateLEDColor(ui8LedData, LED_HB_F2, ui8ColorWhite);
    }
    else
    {
        v_updateLEDColor(ui8LedData, LED_HB_F1, ui8ColorOff);
        v_updateLEDColor(ui8LedData, LED_HB_F2, ui8ColorOff);
    }
}

/* @brief: Updates the brake LEDs based on the current brake status, setting them to red or turning them off.
 *
 * @details: If `bBrakeStatus` is active, the function sets the brake LEDs (`LED_BRK_R1` and `LED_BRK_R2`)
 * to red. Otherwise, it turns them off.
 *
 * @return: no return value
 * @return type: void
 */
void v_updateBrakeLEDs(bool bBrakeStatus, uint8_t (*ui8LedData)[3])
{
    if (bBrakeStatus)
    {
        v_updateLEDColor(ui8LedData, LED_BRK_R1, ui8ColorRed);
        v_updateLEDColor(ui8LedData, LED_BRK_R2, ui8ColorRed);
    }
    else
    {
        v_updateLEDColor(ui8LedData, LED_BRK_R1, ui8ColorOff);
        v_updateLEDColor(ui8LedData, LED_BRK_R2, ui8ColorOff);
    }
}

/* @brief: Updates both left and right turn LEDs simultaneously to indicate hazard status.
 *
 * @details: This function activates or deactivates the hazard lights by updating both the left and right turn LEDs.
 * It calls `v_updateRightTurnLEDs` and `v_updateLeftTurnLEDs` with the provided status (`ui8Status`), ensuring
 * synchronized signaling for hazard indication.
 *
 * @param ui8Status: The status to apply to the hazard LEDs (e.g., ON or OFF).
 *
 * @return: no return value
 * @return type: void
 */
void v_updateHazardLEDs(bool bIsHazardBlinkOn, uint8_t (*ui8LedData)[3])
{
    v_updateRightTurnLEDs(bIsHazardBlinkOn, ui8LedData);
    v_updateLeftTurnLEDs(bIsHazardBlinkOn, ui8LedData);
}

/**
 * @brief: Updates the right turn LEDs based on the given status, setting them to yellow or turning them off.
 *
 * @details: This function controls the right turn signal LEDs (`LED_RD_R` for rear and `LED_RD_F` for front).
 * If `ui8Status` is active, both LEDs are set to yellow to indicate a right turn. Otherwise, they are turned off.
 *
 * @param ui8Status: The status to apply to the right turn LEDs.
 *
 * @return: no return value
 * @return type: void
 */
void v_updateRightTurnLEDs(uint8_t ui8IsRightBlinkOn, uint8_t (*ui8LedData)[3])
{
    if (ui8IsRightBlinkOn)
    {
        v_updateLEDColor(ui8LedData, LED_RD_R, ui8ColorYellow);
        v_updateLEDColor(ui8LedData, LED_RD_F, ui8ColorYellow);
    }
    else
    {
        v_updateLEDColor(ui8LedData, LED_RD_R, ui8ColorOff);
        v_updateLEDColor(ui8LedData, LED_RD_F, ui8ColorOff);
    }
}

/* @brief: Updates the left turn LEDs based on the given status, setting them to yellow or turning them off.
 *
 * @details: This function controls the left turn signal LEDs (`LED_LD_R` for rear and `LED_LD_F` for front).
 * If `ui8Status` is active, both LEDs are set to yellow to indicate a left turn. Otherwise, they are turned off.
 *
 * @param ui8Status: The status to apply to the left turn LEDs (e.g., ON or OFF).
 *
 * @return: no return value
 * @return type: void
 */

void v_updateLeftTurnLEDs(uint8_t ui8IsLeftBlinkOn, uint8_t (*ui8LedData)[3])
{
    if (ui8IsLeftBlinkOn)
    {
        v_updateLEDColor(ui8LedData, LED_LD_R, ui8ColorYellow);
        v_updateLEDColor(ui8LedData, LED_LD_F, ui8ColorYellow);
    }
    else
    {
        v_updateLEDColor(ui8LedData, LED_LD_R, ui8ColorOff);
        v_updateLEDColor(ui8LedData, LED_LD_F, ui8ColorOff);
    }
}
