/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#ifndef __SDK_SWI2C_H
#define __SDK_SWI2C_H

#include "sdk_def.h"
#include "sdk_os_port.h"

#ifdef __cplusplus
extern "C" {
#endif


#define I2C_FLAG_WR             0x0000
#define I2C_FLAG_RD          (1u << 0)
#define I2C_FLAG_ADDR_10BIT  (1u << 2) /* this is a ten bit chip address */
#define I2C_FLAG_NO_START    (1u << 4)
#define I2C_FLAG_IGNORE_NACK (1u << 5)
#define I2C_FLAG_READ_ACK    (1u << 6) /* when I2C reading, we do not ACK */
#define I2C_FLAG_NO_STOP     (1u << 7)

struct i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t  *buf;
};



typedef struct _sdk_swi2c sdk_swi2c_t;

struct sdk_swi2c_ops
{
    void *data;
    sdk_err_t (*init)(void);
    sdk_err_t (*deinit)(void);
    void (*set_sda)(void *data, int32_t state);
    void (*set_scl)(void *data, int32_t state);
    int32_t (*get_sda)(void *data);
    int32_t (*get_scl)(void *data);

    void (*udelay)(uint32_t us);

    uint32_t delay_us;  /* scl and sda line delay */
    uint32_t timeout;   /* in tick */
};

struct _sdk_swi2c
{
    bool is_opened;
    struct sdk_os_mutex lock;
    struct sdk_swi2c_ops *ops;
    int32_t retries;
};

sdk_err_t sdk_swi2c_open(sdk_swi2c_t *bus);
sdk_err_t sdk_swi2c_close(sdk_swi2c_t *bus);
uint32_t sdk_swi2c_write(sdk_swi2c_t *bus, uint16_t addr, const uint8_t *buf, uint32_t count);
uint32_t sdk_swi2c_read(sdk_swi2c_t *bus, uint16_t addr, uint8_t *buf, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
