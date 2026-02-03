/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __SDK_FLASH_H
#define __SDK_FLASH_H

#include "sdk_def.h"
#include "sdk_os_port.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sdk_flash sdk_flash_t;

struct sdk_flash_ops
{
    sdk_err_t (*open)(sdk_flash_t *flash);
    sdk_err_t (*close)(sdk_flash_t *flash);
    int32_t (*read)(sdk_flash_t *flash, uint32_t addr, uint8_t *buf, size_t size);
    int32_t (*write)(sdk_flash_t *flash, uint32_t addr, const uint8_t *buf, size_t size);
    sdk_err_t (*erase)(sdk_flash_t *flash, uint32_t addr, size_t size);
    sdk_err_t (*control)(sdk_flash_t *flash, int32_t cmd, void *args);
};

struct _sdk_flash
{
    bool is_opened;
    struct sdk_os_mutex lock;
    struct sdk_flash_ops ops;
};

#ifdef __cplusplus
}
#endif

#endif  /* __SDK_FLASH_H__ */
