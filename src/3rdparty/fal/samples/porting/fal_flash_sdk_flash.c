/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fal.h>
#include "aft_sdk.h"
#include "sdk_board.h"

static int read(long offset, uint8_t *buf, size_t size)
{
    onchip_flash.ops.read(&onchip_flash, offset, buf, size);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    onchip_flash.ops.write(&onchip_flash, offset, buf, size);

    return size;
}

static int erase(long offset, size_t size)
{
    onchip_flash.ops.erase(&onchip_flash, offset, size);

    return size;
}

const struct fal_flash_dev onchip_flash = {
    .name = "onchip_flash",
    .addr = 0x08000000,
    .len = 1024 * 1024,
    .blk_size = 128 * 1024,
    .ops = {NULL, read, write, erase},
    .write_gran = 32  //  write底层实现是按照32bit实现的
};
