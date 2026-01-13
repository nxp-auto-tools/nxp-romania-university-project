
#ifndef LIGHTS_UTILS_H_
#define LIGHTS_UTILS_H_

#include "WS2811_utils.h"

/* Number of WS2811 LEDs */
#define NUM_LEDS 10

#define LED_STRIP_PORT    PTD
#define LED_STRIP_PIN     6      /* WS2811 pin */

/* @brief: Updates the daylight LEDs based on the current daylight status, setting them to white or turning them off. */
void v_updateDaylightLEDs(bool bDaylightStatus, uint8_t (*ui8LedData)[3]);

/* @brief: Updates the high beam LEDs based on the current high beam status, setting them to white or turning them off. */
void v_updateHighBeamLEDs(bool bHighBeamStatus, uint8_t (*ui8LedData)[3]);

/* @brief: Updates the brake LEDs based on the current brake status, setting them to red or turning them off. */
void v_updateBrakeLEDs(bool bBrakeStatus, uint8_t (*ui8LedData)[3]);

/* @brief: Updates both left and right turn LEDs simultaneously to indicate hazard status. */
void v_updateHazardLEDs(bool bIsHazardBlinkOn, uint8_t (*ui8LedData)[3]);

/* @brief: Updates the right turn LEDs based on the given status, setting them to yellow or turning them off. */
void v_updateRightTurnLEDs(uint8_t ui8IsRightBlinkOn, uint8_t (*ui8LedData)[3]);

/* @brief: Updates the left turn LEDs based on the given status, setting them to yellow or turning them off. */
void v_updateLeftTurnLEDs(uint8_t ui8IsLeftBlinkOn, uint8_t (*ui8LedData)[3]);

#endif /* LIGHTS_UTILS_H_ */
