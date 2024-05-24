/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fal.h>
#include "aft_sdk.h"
#include "sdk_board.h"

#ifndef FAL_USING_ONCHIP_FLASH_DEV_NAME
#define FAL_USING_ONCHIP_FLASH_DEV_NAME             "onchip_flash"
#endif

static int read(long offset, uint8_t *buf, size_t size)
{
    onchip_flash.ops.read(&onchip_flash, MCU_FLASH_START_ADRESS + offset, buf, size);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    onchip_flash.ops.write(&onchip_flash, MCU_FLASH_START_ADRESS + offset, buf, size);

    return size;
}

static int erase(long offset, size_t size)
{
    onchip_flash.ops.erase(&onchip_flash, MCU_FLASH_START_ADRESS + offset, size);

    return size;
}

const struct fal_flash_dev onchip_flash0 = {
    .name = FAL_USING_ONCHIP_FLASH_DEV_NAME,
    .addr = MCU_FLASH_START_ADRESS,
    .len = MCU_FLASH_SIZE,
    .blk_size = MCU_FLASH_PAGE_SIZE,
    .ops = {NULL, read, write, erase},
    .write_gran = 32  //  write底层实现是按照32bit实现的
};
