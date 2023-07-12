/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#include "sdk_board.h"
#include "upm.h"

#define DBG_LVL DBG_LOG
#define DBG_TAG "upm.onchip"
#include "sdk_log.h"

static int init(void)
{
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    return gd32f30x_onchip_flash.ops.read(&gd32f30x_onchip_flash, upm_flash_gd32.addr + offset, buf, size);
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    return gd32f30x_onchip_flash.ops.write(&gd32f30x_onchip_flash, upm_flash_gd32.addr + offset, buf, size);
}

static int erase(long offset, size_t size)
{
    return gd32f30x_onchip_flash.ops.erase(&gd32f30x_onchip_flash, upm_flash_gd32.addr + offset, size);
}

const struct upm_stor_dev upm_flash_gd32 ={
        .name = "gd32_onchip",
        .addr = 0x08000000,
        .len = 128 * 1024,
        .blk_size = 1 * 1024,
        .ops.init = init,
        .ops.read = read,
        .ops.write = write,
        .ops.erase = erase,
};
