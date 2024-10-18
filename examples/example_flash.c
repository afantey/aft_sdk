/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-04-13     rgw             first version
 */

#include "aft_sdk.h"
#include "sdk_uart.h"
#include "sdk_board.h"
#include "fal.h"

#define DBG_LVL DBG_LOG
#define DBG_TAG "test_flash"
#include "sdk_log.h"

#define TEST_DATA_SIZE  1024

uint8_t write_buf[TEST_DATA_SIZE];
uint8_t read_buf[TEST_DATA_SIZE];

int testflash(void)
{
    board_init();
    fal_init();
    LOG_D("hello example flash");

    const struct fal_partition *fal_part = fal_partition_find("primary");
    if (fal_part == NULL) {
        LOG_E("partition not found");
        return -1;
    }

    // 测试擦除
    LOG_D("erase flash");
    fal_partition_erase(fal_part, 0, fal_part->len);

    // 测试写入
    LOG_D("write data to flash");
    memset(write_buf, 0x55, TEST_DATA_SIZE);
    int ret = fal_partition_write(fal_part, 0, write_buf, TEST_DATA_SIZE);
    if (ret < 0) {
        LOG_E("write flash failed");
        return -1;
    }

    // 测试读取
    LOG_D("read data from flash");
    ret = fal_partition_read(fal_part, 0, read_buf, TEST_DATA_SIZE);
    if (ret < 0) {
        LOG_E("read flash failed");
        return -1;
    }

    // 判断写入和读取数据是否正确
    LOG_D("compare data");
    int i;
    for (i = 0; i < TEST_DATA_SIZE; i++) {
        if (write_buf[i] != read_buf[i]) {
            LOG_E("data not match");
            return -1;
        }
    }

    LOG_D("test success");

    return 0;
}
