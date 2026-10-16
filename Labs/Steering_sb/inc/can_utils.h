/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef CAN_UTILS_H
#define CAN_UTILS_H

#include "sdk_project_config.h"

/* Definition of the TX and RX message id's*/

#define HMI_TX_ID           0x110
#define BRAKES_TX_ID        0x111
#define COMFORT_TX_ID      0x112
#define LIGHTS_TX_ID        0x113
#define STEERING_TX_ID      0x114
#define TRANSMISSION_TX_ID  0x115

#define HMI_RX_ID           0x210
#define BRAKES_RX_ID        0x211
#define COMFORT_RX_ID      0x212
#define LIGHTS_RX_ID        0x213
#define STEERING_RX_ID      0x214
#define TRANSMISSION_RX_ID  0x215

//status_t CAN_SendData(uint8_t mailbox,
//                      uint32_t msg_id,
//                      uint8_t txData[],
//                      flexcan_data_info_t *txInfo);

#endif
