/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 */

#include "sdk_project_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "can_utils.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

#define welcomeStr "Welcome to HMI console debugging!"
#define CHECK_BIT(x, pos) (x & (1UL << pos) )

/* MB indices for each message ID */
#define RX_MB_BRAKES       1U
#define RX_MB_COMFORT      2U
#define RX_MB_LIGHTS       3U
#define RX_MB_STEERING     4U
#define RX_MB_TRANSMISSION 5U

volatile uint32_t rx_mask = 0;
flexcan_msgbuff_t dataBuffRx[6]; /* Separate buffers for each MB */
volatile int exit_code = 0;

enum {
    BTNFLAG_DOWN = 0, BTNFLAG_UP = 1, FLAG_LOCK = 2, /* Lock for stepper motor*/
    FLAG_CLIM = 3 /* Climate system state*/
};


/******************************************************************************
 * Function prototypes
 ******************************************************************************/

void Init(void);

void flexcanCallback(uint8_t instance,
                     flexcan_event_type_t eventType,
                     uint32_t buffIdx,
                     flexcan_state_t *flexcanState);
void uartSendTransmission(flexcan_msgbuff_t dataBuff);

static status_t nextion_send(const char *cmd);

void uartSendSteering(flexcan_msgbuff_t dataBuff);

void uartSendBrakes(flexcan_msgbuff_t dataBuff);

void uartSendComfort(flexcan_msgbuff_t dataBuff);

void uartSendLights(flexcan_msgbuff_t dataBuff) ;


int main(void) {

    Init();

    FLEXCAN_DRV_InstallEventCallback(INST_FLEXCAN, &flexcanCallback, NULL);

    /* 
     * TODO 2: Start receiving data on each configured message buffer.
     * Hint: Use the function FLEXCAN_DRV_Receive(uint8_t instance, uint8_t mb_idx, flexcan_msgbuff_t *data).
     * This will enable the interrupt for each buffer so that the callback can be triggered.
     * You should start receiving for the 5 Message Buffers (MBs) we configured in Init().
     */


//    printf(welcomeStr);

    while (1) {
        if (rx_mask != 0) {
            /* Priority 1: Brakes and Lights (High) */
            if (rx_mask & (1U << RX_MB_BRAKES)) {
                uartSendBrakes(dataBuffRx[RX_MB_BRAKES]);
                rx_mask &= ~(1U << RX_MB_BRAKES);
                FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MB_BRAKES, &dataBuffRx[RX_MB_BRAKES]);
            }
            if (rx_mask & (1U << RX_MB_LIGHTS)) {
                uartSendLights(dataBuffRx[RX_MB_LIGHTS]);
                rx_mask &= ~(1U << RX_MB_LIGHTS);
                FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MB_LIGHTS, &dataBuffRx[RX_MB_LIGHTS]);
            }

            /* Priority 2: Steering and Comfort (Middle) */
            if (rx_mask & (1U << RX_MB_STEERING)) {
                uartSendSteering(dataBuffRx[RX_MB_STEERING]);
                rx_mask &= ~(1U << RX_MB_STEERING);
                FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MB_STEERING, &dataBuffRx[RX_MB_STEERING]);
            }
            if (rx_mask & (1U << RX_MB_COMFORT)) {
                uartSendComfort(dataBuffRx[RX_MB_COMFORT]);
                rx_mask &= ~(1U << RX_MB_COMFORT);
                FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MB_COMFORT, &dataBuffRx[RX_MB_COMFORT]);
            }

            /* Priority 3: Transmission (Low) */
            if (rx_mask & (1U << RX_MB_TRANSMISSION)) {
                uartSendTransmission(dataBuffRx[RX_MB_TRANSMISSION]);
                rx_mask &= ~(1U << RX_MB_TRANSMISSION);
                FLEXCAN_DRV_Receive(INST_FLEXCAN, RX_MB_TRANSMISSION, &dataBuffRx[RX_MB_TRANSMISSION]);
            }
        }
    }
    return exit_code;
}

/******************************************************************************
 * Functions
 ******************************************************************************/

/*
 * @brief : Initialize clocks, pins and power modes
 */
void Init(void) {

    /* Configure clocks for PORT */
    CLOCK_DRV_Init(&clockMan1_InitConfig0);

    /* Initialize pins
     *  -   Init FlexCAN and GPIO pins
     *  -   See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    /*
     * TODO 0: Initialize the FlexCAN module.
     * Hint: Use the function FLEXCAN_DRV_Init(uint8_t instance, flexcan_state_t *state, const flexcan_user_config_t *config).
     * You should use 'INST_FLEXCAN', '&flexcanState0', and '&flexcanInitConfig0'.
     */

    /* Configure RX MBs with their respective IDs */
    flexcan_data_info_t rxInfo = {
        .msg_id_type = FLEXCAN_MSG_ID_STD,
        .data_length = 8U,
        .is_remote = false
    };

    /* 
     * TODO 1: Configure the receive message buffers.
     * Hint: Use the function FLEXCAN_DRV_ConfigRxMb(uint8_t instance, uint8_t mb_idx, const flexcan_data_info_t *rx_info, uint32_t msg_id).
     * You need to configure 5 buffers for the different messages: 
     * - BRAKES_TX_ID (RX_MB_BRAKES)
     * - COMFORT_TX_ID (RX_MB_COMFORT)
     * - LIGHTS_TX_ID (RX_MB_LIGHTS)
     * - STEERING_TX_ID (RX_MB_STEERING)
     * - TRANSMISSION_TX_ID (RX_MB_TRANSMISSION)
     */



    if (LPUART_DRV_Init(INST_LPUART_1, &lpUartState0, &lpuart_0_InitConfig0) != STATUS_SUCCESS)
        __asm("bkpt #255");
    OSIF_TimeDelay(200);

}

void flexcanCallback(uint8_t instance,
                     flexcan_event_type_t eventType,
                     uint32_t buffIdx,
                     flexcan_state_t *flexcanState) {
    (void) flexcanState;
    (void) instance;

    switch (eventType) {
    case FLEXCAN_EVENT_RX_COMPLETE:
        /* 
         * TODO 3: Implement the FlexCAN callback logic for RX.
         * Hint: Check if the eventType is FLEXCAN_EVENT_RX_COMPLETE.
         * If it is, update the 'rx_mask' by setting the bit corresponding to 'buffIdx'.
         * Pattern: rx_mask |= (1U << buffIdx); 
         * This flag will alert the main loop to process the new data.
         */
        break;


    case FLEXCAN_EVENT_TX_COMPLETE:
        break;
    default:
        break;
    }
}


static status_t nextion_send(const char *cmd)
{
    uint8_t out[64];
    size_t len = strlen(cmd);
    if (len + 3 > sizeof(out)) return STATUS_ERROR;

    memcpy(out, cmd, len);
    out[len++] = 0xFF;
    out[len++] = 0xFF;
    out[len++] = 0xFF;

    return LPUART_DRV_SendDataBlocking(INST_LPUART_1, out, (uint32_t)len, 50U);
}

void uartSendTransmission(flexcan_msgbuff_t dataBuff) {
    uint8_t  ui8Gear        = dataBuff.data[0];
    uint16_t ui16CurrentRpm = (((uint16_t)dataBuff.data[1] << 8) | dataBuff.data[2])/10;
    uint8_t  ui8Speed       = dataBuff.data[3];

    char cmd[50];

    snprintf(cmd, sizeof(cmd), "Gear.txt=\"%u\"", (unsigned)ui8Gear);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "rpm.val=%u", (unsigned)ui16CurrentRpm);

    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "speed.val=%u", (unsigned)ui8Speed);
    nextion_send(cmd);
}

void uartSendSteering(flexcan_msgbuff_t dataBuff) {
    char cmd[50];
    uint8_t  ui8Angle = dataBuff.data[0];

    snprintf(cmd, sizeof(cmd), "steer.val=%u", (unsigned)ui8Angle);
    nextion_send(cmd);
    //printf("steer.val=%u", (unsigned)ui8Angle);

}

void uartSendBrakes(flexcan_msgbuff_t dataBuff) {
    char cmd[50];
    uint8_t  ui8LedsState = dataBuff.data[0];

    snprintf(cmd, sizeof(cmd), "ledsState.val=%u", (unsigned)ui8LedsState);
    nextion_send(cmd);

}

void uartSendComfort(flexcan_msgbuff_t dataBuff) {
    uint8_t  ui8CurrentTemp = dataBuff.data[0];
    uint8_t ui8TargetTemp = dataBuff.data[1];
    uint8_t  ui8_CurrentFlags = dataBuff.data[2];

    bool bFlagClima = CHECK_BIT(ui8_CurrentFlags, FLAG_CLIM);
    bool bFlagLock  = CHECK_BIT(ui8_CurrentFlags, FLAG_LOCK);
    bool bFlagDown  = CHECK_BIT(ui8_CurrentFlags, BTNFLAG_DOWN);
    bool bFlagUp = CHECK_BIT(ui8_CurrentFlags, BTNFLAG_UP);

    char cmd[50];

    snprintf(cmd, sizeof(cmd), "flagLock.val=%u", bFlagLock?1:0);
//    printf("flagLock.val=%u", bFlagLock?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "flagClima.val=%u", bFlagClima?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "flagDown.val=%u", bFlagDown?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "flagUp.val=%u", bFlagUp?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "currentTemp.val=%u", (unsigned)ui8CurrentTemp);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "targetTemp.val=%u", (unsigned)ui8TargetTemp);
    nextion_send(cmd);

}

void uartSendLights(flexcan_msgbuff_t dataBuff) {
    uint8_t rxData = dataBuff.data[0];

    bool bDaylightStatus   = CHECK_BIT(rxData, 1);
    bool bHighBeamStatus   = CHECK_BIT(rxData, 2);
    bool bBrakeStatus      = CHECK_BIT(rxData, 3);
    bool bIsRightBlinkOn   = CHECK_BIT(rxData, 4);
    bool bIsLeftBlinkOn    = CHECK_BIT(rxData, 5);
    bool bIsHazardBlinkOn  = CHECK_BIT(rxData, 6);


    char cmd[50];

    snprintf(cmd, sizeof(cmd), "bDaylight.val=%u", bDaylightStatus?1:0);

    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "bHighBeam.val=%u", bHighBeamStatus?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "bBrake.val=%u", bBrakeStatus?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "bIsRightBlink.val=%u", bIsRightBlinkOn?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "bIsLeftBlink.val=%u", bIsLeftBlinkOn?1:0);
    nextion_send(cmd);

    snprintf(cmd, sizeof(cmd), "bIsHazardBlink.val=%u", bIsHazardBlinkOn?1:0);
    nextion_send(cmd);

}
