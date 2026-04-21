/*
 * Copyright (c) 2015 - 2016 , Freescale Semiconductor, Inc.
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 */
#include "WS2811_utils.h"
#include "lights_utils.h"
#include "can_utils.h"

// pin port configuration
#define LED0_PORT PTD
#define LED0_PIN 15 // RGB_RED
#define LED1_PORT PTD
#define LED1_PIN 16 // RGB_GREEN
#define LED2_PORT PTD
#define LED2_PIN 0 // RGB_BLUE

#define BUTTON_LEFT_TURN_PORT PTA
#define BUTTON_LEFT_TURN_PIN 17
#define BUTTON_RIGHT_TURN_PORT PTD
#define BUTTON_RIGHT_TURN_PIN 10
#define BUTTON_DAYLIGHT_PORT PTD
#define BUTTON_DAYLIGHT_PIN 3
#define BUTTON_HIGH_BEAM_PORT PTD
#define BUTTON_HIGH_BEAM_PIN 5
#define BUTTON_BRAKE_TURN_PORT PTD
#define BUTTON_BRAKE_TURN_PIN 11
#define BUTTON_GREEN_PORT PTD
#define BUTTON_GREEN_PIN 12

#define DELAY_TIME 2000000
#define LED_UPDATE_DELAY 1520000

//FlexCAN configurations

/* Variables for FlexCAN transmission */

#define DAYLIGHT_BIT_OFFSET 1
#define HIGHBEAM_BIT_OFFSET 2
#define BRAKE_BIT_OFFSET 3
#define RIGHTBLINK_BIT_OFFSET 4
#define LEFTBLINK_BIT_OFFSET 5
#define HAZARD_BIT_OFFSET 6

uint8_t txData[1] = { 0 };
uint8_t previousTxData = 0;

/* Usefull Macros */
#define SET_BIT(val, pos) (val << pos)
#define TX_MAILBOX 9
#define RX_MAILBOX 8

/* Message buffer configuration */
static flexcan_data_info_t txInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U

};

static flexcan_data_info_t rxInfo = { .data_length = 1U,
                                      .msg_id_type = FLEXCAN_MSG_ID_STD,
                                      .enable_brs = false,
                                      .fd_enable = false,
                                      .fd_padding = 0U
};

// For blinking, left and right turn shall be 3 times OFF and 3 times ON
#define TURN_CYCLES 6

#define MAX_COUNTER 2

int iBlinkCounter = 0;

bool bIsRightBlinkOn = false;
bool bIsLeftBlinkOn = false;
bool bIsHazardBlinkOn = false;

// State variables
bool bDaylightStatus = 0;
bool bHighBeamStatus = 0;
uint8_t ui8RightTurnStatus = 0;
uint8_t ui8LeftTurnStatus = 0;
bool bBrakeStatus = 0;
bool bHazardStatus = 0;

// Arrays for LED colors, each LED has 3 bytes (G, R, B)
uint8_t ui8LedData[NUM_LEDS][3] = {0};

/* Function prototypes ------------------------------------------------------------ */

/* @brief: Sequentially tests each LED by turning it on and off. */
void v_checkLEDs(void);

/* @brief: Resets the LED blink counter to zero. */
void v_resetCounter();

/* @brief: Checks if a button connected to a specific port and pin is pressed.  */
bool b_checkButtonPressed(uint32_t ui32Port, uint8_t ui32Pin);

/* @brief: Updates all PCB LEDs based on system status flags.*/
void v_updatePCBLEDs();

/* @brief: Populates the tx buffer based on system status flags.*/
void populateCANTXBuffer();

flexcan_msgbuff_t rx_msg;
uint8_t brakesIntensity = 0;

void flexcanCallback(uint8_t instance,
                     flexcan_event_type_t eventType,
                     uint32_t buffIdx,
                     flexcan_state_t *flexcanState) {
    if(eventType == FLEXCAN_EVENT_RX_COMPLETE && buffIdx == RX_MAILBOX){
        /* 
         * TODO 3: Implement the FlexCAN callback logic for RX.
         * Hint: Check if the eventType is FLEXCAN_EVENT_RX_COMPLETE.
         * If it is, update the 'brakesIntensity' from rx_msg.data[0].
         * You also need to re-arm the RX buffer using FLEXCAN_DRV_Receive().
         */
    }

}


int main(void)
{
    status_t error;

    /* Configure clocks for PORT */
    error = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    DEV_ASSERT(error == STATUS_SUCCESS);
    /* Set pins as GPIO */
    error = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    DEV_ASSERT(error == STATUS_SUCCESS);

    /*
     * TODO 0: Initialize the FlexCAN module.
     * Hint: Use the function FLEXCAN_DRV_Init(uint8_t instance, flexcan_state_t *state, const flexcan_user_config_t *config).
     * You should use 'INST_FLEXCAN_CONFIG_1', '&flexcanState0', and '&flexcanInitConfig0'.
     */

    /* 
     * TODO 4: Configure the TX message buffer for sending light data.
     * Hint: Use the function FLEXCAN_DRV_ConfigTxMb(instance, mb_idx, &tx_info, msg_id).
     */

    v_updateLEDsAllOff(ui8LedData);
    //  check all LEDs are working
    v_checkLEDs();

    v_delayCycles(LED_UPDATE_DELAY);

    /*  Installs a callback function for the FlexCAN IRQ handler  */
    FLEXCAN_DRV_InstallEventCallback(INST_FLEXCAN_CONFIG_1, &flexcanCallback, NULL);

    /* 
     * TODO 1: Configure the RX message buffer for receiving brake intensity.
     * Hint: Use the function FLEXCAN_DRV_ConfigRxMb(instance, mb_idx, &rx_info, msg_id).
     */

    /* 
     * TODO 2: Start receiving data on the configured mailbox.
     * Hint: Use the function FLEXCAN_DRV_Receive(instance, mb_idx, &rx_msg).
     */





    // Infinite loop to continuously check button states and update LEDs
    while (1)
    {

        // Toggle daylight status on button press
        if (b_checkButtonPressed(BUTTON_DAYLIGHT_PORT->PDIR,
        		BUTTON_DAYLIGHT_PIN))
        {
            bDaylightStatus ^= 1;
        }

        // If daylight status is OFF, ensure high beam is also OFF
        if (!bDaylightStatus)
        {
            bHighBeamStatus = 0;
        }

        // Toggle high beam status if daylight is on and button is pressed
        if (bDaylightStatus && b_checkButtonPressed(BUTTON_HIGH_BEAM_PORT->PDIR,
        		BUTTON_HIGH_BEAM_PIN))
        {
            bHighBeamStatus ^= 1;
        }

        /* Determine the braking status for local LEDs (Standalone + Low Latency) */
        /* It is active if the local physical button is pressed OR if the remote Brakes board signal is active */
        if (b_checkButtonPressed(BUTTON_BRAKE_TURN_PORT->PDIR, BUTTON_BRAKE_TURN_PIN) || (brakesIntensity > 0))
        {
            bBrakeStatus = 1;
        }
        else
        {
            bBrakeStatus = 0;
        }

        // Check hazard button state and update hazard status accordingly
        if (b_checkButtonPressed(BUTTON_GREEN_PORT->PDIR, BUTTON_GREEN_PIN))
        {
            bHazardStatus ^= 1;
            bIsHazardBlinkOn = true;

            // Turn OFF right and left turn signal
            // Prevents turn signals from blinking after hazard mode is turned off
            ui8RightTurnStatus = 0;
            ui8LeftTurnStatus = 0;

            v_resetCounter();
        }

        // Check right turn signal button state and update LEDs accordingly
        if (b_checkButtonPressed(BUTTON_RIGHT_TURN_PORT->PDIR,
                                 BUTTON_RIGHT_TURN_PIN) &&
            !bHazardStatus)
        {
            // Activate right turn signal
            ui8RightTurnStatus = TURN_CYCLES;

            // Deactivate left turn signal
            ui8LeftTurnStatus = 0;
        }

        // Check left turn signal button state and update LEDs accordingly
        if (b_checkButtonPressed(BUTTON_LEFT_TURN_PORT->PDIR,
                                 BUTTON_LEFT_TURN_PIN) &&
            !bHazardStatus)
        {
            // Activate left turn signal
            ui8LeftTurnStatus = TURN_CYCLES;

            // Deactivate right turn signal
            ui8RightTurnStatus = 0;
        }

        // Update the state of all car LEDs based on the current status variables
        v_updatePCBLEDs();

        populateCANTXBuffer();

        /* 
         * TODO 5: Send the CAN message with the lights data.
         * Hint: Use the function CAN_SendData(uint8_t mailbox, uint32_t msg_id, uint8_t txData[], flexcan_data_info_t *txInfo).
         * Make sure the CAN TX message buffer is not busy before sending.
         */


        // Send the updated LED data to the hardware
        v_sendLEDData(ui8LedData, NUM_LEDS);

        // If counter reaches maximum, reset; otherwise, increment
        iBlinkCounter == MAX_COUNTER ? v_resetCounter() : iBlinkCounter++;

        v_delayCycles(LED_UPDATE_DELAY);
    }
}

/* Functions ---------------------------------------------------------------------- */

/* @brief: Sequentially tests each LED by turning it on and off.
 * This function iterates through all LEDs, turning each one on with white color,
 * then off, with a delay between each step.
 *
 * @return: no return value
 * @return type: void
 */
void v_checkLEDs(void)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        v_updateLEDColor(ui8LedData, i, ui8ColorWhite);
        v_sendLEDData(ui8LedData, NUM_LEDS);
        v_delayCycles(DELAY_TIME);
    }
    for (int i = 0; i < NUM_LEDS; i++)
    {
        v_updateLEDColor(ui8LedData, i, ui8ColorOff);
        v_sendLEDData(ui8LedData, NUM_LEDS);
        v_delayCycles(DELAY_TIME);
    }
}

/* @brief: Resets the LED blink counter to zero.
 *
 * @return: no return value
 * @return type: void
 */
void v_resetCounter()
{
    iBlinkCounter = 0;
}

/* @brief: Checks if a button connected to a specific port and pin is pressed.
 *
 * @param ui32Port: digital input register of the port (e.g., PTE->PDIR).
 * @param type: uint32_t
 *
 * @param ui32Pin: pin number where the button is connected.
 * @param type: uint8_t
 *
 * @return: true if the button is pressed, false otherwise.
 * @return type: bool
 */
bool b_checkButtonPressed(uint32_t ui32Port, uint8_t ui32Pin)
{
    return !(ui32Port & (1 << ui32Pin));
}

/* @brief: Updates all PCB LEDs based on system status flags.
 *
 * @details: This function coordinates the update of all relevant LEDs on the PCB, including:
 * - Daylight LEDs - `v_updateDaylightLEDs()`
 * - High beam LEDs - `v_updateHighBeamLEDs()`
 * - Brake LEDs - `v_updateBrakeLEDs()`
 * - Hazard and turn signal LEDs based on their respective status flags:
 *   - If `bHazardStatus` is active, hazard LEDs blink based on `ui8HazardBlinkState`,
 *    toggled when `iBlinkCounter` reaches `MAX_COUNTER`.
 *   - If `bRightTurnStatus` is active, right turn LEDs blink while left turn LEDs are turned off.
 *   - If `bLeftTurnStatus` is active, left turn LEDs blink while right turn LEDs are turned off.
 *   - If no turn or hazard status is active, both turn LEDs are turned off.
 *
 * The function also handles blink state toggling and counter resets for timed LED blinking.
 *
 * @return: no return value
 * @return type: void
 */
void v_updatePCBLEDs()
{
    // Update LED colors based on daylight status
    v_updateDaylightLEDs(bDaylightStatus, ui8LedData);

    // Update high beam LEDs based on high beam status
    v_updateHighBeamLEDs(bHighBeamStatus, ui8LedData);

    // Update Brake LEDs based on brake status
    v_updateBrakeLEDs(bBrakeStatus, ui8LedData);

    // Update hazard LEDs based on hazard state
    if (bHazardStatus)
    {
        v_updateHazardLEDs(bIsHazardBlinkOn,ui8LedData);
        if (iBlinkCounter == MAX_COUNTER)
        {
            // Change the state of hazard LEDs when MAX_COUNTER is reached
            bIsHazardBlinkOn ^= 1;
            v_resetCounter();
        }
    }
    else if (ui8RightTurnStatus)
    {
        v_updateLeftTurnLEDs(false, ui8LedData);
        v_updateRightTurnLEDs(bIsRightBlinkOn, ui8LedData);
        if (iBlinkCounter == MAX_COUNTER)
        {
            // Change the state of right blink LEDs when MAX_COUNTER is reached
            bIsRightBlinkOn ^= 1;
            v_resetCounter();
            ui8RightTurnStatus--;
        }
    }
    else if (ui8LeftTurnStatus)
    {
        v_updateRightTurnLEDs(false,ui8LedData);
        v_updateLeftTurnLEDs(bIsLeftBlinkOn,ui8LedData);
        if (iBlinkCounter == MAX_COUNTER)
        {
            // Change the state of left blink LEDs when MAX_COUNTER is reached
            bIsLeftBlinkOn ^= 1;
            v_resetCounter();
            ui8LeftTurnStatus--;
        }
    }
    else
    {
        v_updateRightTurnLEDs(false,ui8LedData);
        v_updateLeftTurnLEDs(false,ui8LedData);
    }
}

/* @brief: Populates the FlexCAN TX message buffer based on system status flags
 *
 * @details : This function constructs a CAN message payload by setting individual bits by using the SET_BIT macro.
 *
 * @return: no return value
 * @return type: void
 */
void populateCANTXBuffer(){
    /* 
     * TODO 6: Populate the transmission data for the HMI project.
     * Hint: Use the SET_BIT macro to pack the status flags into 'txData[0]'. Check the HMI project to see what data it expects.
     */
}


