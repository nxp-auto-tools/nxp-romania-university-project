/*
* Copyright 2026 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "can_utils.h"

status_t CAN_SendData(uint8_t mailbox,
                      uint32_t msg_id,
                      uint8_t txData[],
                      flexcan_data_info_t* txInfo) {
    status_t stat;
    do {
        stat = FLEXCAN_DRV_Send(INST_FLEXCAN,
                                mailbox,
                                txInfo,
                                msg_id,
                                txData);

    } while (stat != STATUS_SUCCESS);
    return stat;
}
