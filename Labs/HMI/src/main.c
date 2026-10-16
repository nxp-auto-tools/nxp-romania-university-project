/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
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

volatile bool dataReceived;
flexcan_msgbuff_t dataBuffRx;
volatile int exit_code = 0;

enum {
    BTNFLAG_DOWN = 0, BTNFLAG_UP = 1, FLAG_LOCK = 2, /* Lock for stepper motor*/
    FLAG_CLIM = 3 /* Climate system state*/
};


//TODO: Define the CAN filter table (IDs + frame configuration)
const flexcan_id_table_t filterTable[] = { };

/******************************************************************************
 * Function prototypes
 ******************************************************************************/

void Init(void);

void uartSend(flexcan_msgbuff_t dataBuff);

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

	//TODO: Initialize the system
	//TODO: Register the FlexCAN event callback (FLEXCAN_DRV_InstallEventCallback)
	//TODO: Start the first Rx FIFO reception (FLEXCAN_DRV_RxFifo)
	//TODO: send the received CAN message via UART to the HMI

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

    FLEXCAN_DRV_Init(INST_FLEXCAN, &flexcanState0, &flexcanInitConfig0);

    FLEXCAN_DRV_ConfigRxFifo(INST_FLEXCAN,
                             FLEXCAN_RX_FIFO_ID_FORMAT_A,
                             filterTable);

    if (LPUART_DRV_Init(INST_LPUART_1, &lpUartState0, &lpuart_0_InitConfig0) != STATUS_SUCCESS)
                __asm("bkpt #255");

        LPUART_DRV_SetBaudRate(INST_LPUART_1, 9600U);
        OSIF_TimeDelay(200);

}

void flexcanCallback(uint8_t instance,
                     flexcan_event_type_t eventType,
                     uint32_t buffIdx,
                     flexcan_state_t *flexcanState) {
    (void) flexcanState;
    (void) instance;
    (void) buffIdx;

    switch (eventType) {
    case FLEXCAN_EVENT_RXFIFO_COMPLETE:
        dataReceived = true;
        break;
    case FLEXCAN_EVENT_TX_COMPLETE:
        break;
    default:
        break;
    }
}

void uartSend(flexcan_msgbuff_t dataBuff) {
    switch (dataBuffRx.msgId) {
                case BRAKES_TX_ID:
//                    printf("B\n");
                    uartSendBrakes(dataBuffRx);
                    break;
                case COMFORT_TX_ID:
//                    printf("C\n");
                    uartSendComfort(dataBuffRx);
                    break;
                case LIGHTS_TX_ID:
//                    printf("L\n");
                    uartSendLights(dataBuffRx);
                    break;
                case STEERING_TX_ID:
//                    printf("S\n");
                    uartSendSteering(dataBuffRx);
                    break;
                case TRANSMISSION_TX_ID:
//                    printf("T\n");
                    uartSendTransmission(dataBuffRx);
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
//    printf("steer.val=%u", (unsigned)ui8Angle);

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
