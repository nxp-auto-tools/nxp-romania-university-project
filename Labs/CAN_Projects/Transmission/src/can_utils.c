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
