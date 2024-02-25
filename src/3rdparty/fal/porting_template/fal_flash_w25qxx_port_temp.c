/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-01-19     rgw             first version
 */

#include "sdk_board.h"
#include "fal.h"
#include "w25qxx.h"

#ifndef FAL_USING_NOR_FLASH_DEV_NAME
#define FAL_USING_NOR_FLASH_DEV_NAME             "norflash0"
#endif

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

struct fal_flash_dev nor_flash0 = {
    .name = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr = 0,
    // .len = 0,
    // .blk_size = 0,
    .ops = {init, read, write, erase},
    .write_gran = 1
};

static int init(void)
{
    /* update the flash chip information */
    nor_flash0.blk_size = w25qxx.sector_size;
    nor_flash0.len = w25qxx.block_size * w25qxx.block_count;

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    w25qxx_read(&w25qxx, nor_flash0.addr + offset, buf, size);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    if (w25qxx_write(&w25qxx, nor_flash0.addr + offset, (uint8_t *)buf, size) != W25QXX_Ok)
    {
        return -1;
    }

    return size;
}

static int erase(long offset, size_t size)
{
    if (w25qxx_erase(&w25qxx, nor_flash0.addr + offset, size) != W25QXX_Ok)
    {
        return -1;
    }

    return size;
}
