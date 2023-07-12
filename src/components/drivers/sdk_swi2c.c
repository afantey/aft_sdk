/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_board.h"
#include "sdk_swi2c.h"

#define DBG_TAG "I2C"
#ifdef I2C_BITOPS_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif

#include "sdk_log.h"

#define SET_SDA(ops, val) ops->set_sda(ops->data, val)
#define SET_SCL(ops, val) ops->set_scl(ops->data, val)
#define GET_SDA(ops)      ops->get_sda(ops->data)
#define GET_SCL(ops)      ops->get_scl(ops->data)

void i2c_delay(struct sdk_swi2c_ops *ops)
{
    ops->udelay((ops->delay_us + 1) >> 1);
}

void i2c_delay2(struct sdk_swi2c_ops *ops)
{
    ops->udelay(ops->delay_us);
}

#define SDA_L(ops) SET_SDA(ops, 0)
#define SDA_H(ops) SET_SDA(ops, 1)
#define SCL_L(ops) SET_SCL(ops, 0)

/**
 * release scl line, and wait scl line to high.
 */
static sdk_err_t SCL_H(struct sdk_swi2c_ops *ops)
{
    uint32_t start;

    SET_SCL(ops, 1);

    if (!ops->get_scl)
        goto done;

    start = sdk_hw_get_systick();
    while (!GET_SCL(ops))
    {
        if ((sdk_hw_get_systick() - start) > ops->timeout)
            return -SDK_E_TIMEOUT;
        i2c_delay(ops);
    }
#ifdef I2C_BITOPS_DEBUG
    if (sdk_hw_get_systick() != start)
    {
        LOG_D("wait %ld tick for SCL line to go high",
              sdk_hw_get_systick() - start);
    }
#endif

done:
    i2c_delay(ops);

    return SDK_OK;
}

static void i2c_start(struct sdk_swi2c_ops *ops)
{
#ifdef I2C_BITOPS_DEBUG
    if (ops->get_scl && !GET_SCL(ops))
    {
        LOG_E("I2C bus error, SCL line low");
    }
    if (ops->get_sda && !GET_SDA(ops))
    {
        LOG_E("I2C bus error, SDA line low");
    }
#endif
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_restart(struct sdk_swi2c_ops *ops)
{
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_stop(struct sdk_swi2c_ops *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_H(ops);
    i2c_delay2(ops);
}

static bool i2c_waitack(struct sdk_swi2c_ops *ops)
{
    bool ack;

    SDA_H(ops);
    i2c_delay(ops);

    if (SCL_H(ops) < 0)
    {
        LOG_W("wait ack timeout");

        return -SDK_E_TIMEOUT;
    }

    ack = !GET_SDA(ops); /* ACK : SDA pin is pulled low */
    LOG_D("%s", ack ? "ACK" : "NACK");

    SCL_L(ops);

    return ack;
}

static int32_t i2c_writeb(sdk_swi2c_t *bus, uint8_t data)
{
    int32_t i;
    uint8_t bit;

    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;

    for (i = 7; i >= 0; i--)
    {
        SCL_L(ops);
        bit = (data >> i) & 1;
        SET_SDA(ops, bit);
        i2c_delay(ops);
        if (SCL_H(ops) < 0)
        {
            LOG_D("i2c_writeb: 0x%02x, "
                  "wait scl pin high timeout at bit %d",
                  data, i);

            return -SDK_E_TIMEOUT;
        }
    }
    SCL_L(ops);
    i2c_delay(ops);

    return i2c_waitack(ops);
}

static int32_t i2c_readb(sdk_swi2c_t *bus)
{
    uint8_t i;
    uint8_t data = 0;
    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;

    SDA_H(ops);
    i2c_delay(ops);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        if (SCL_H(ops) < 0)
        {
            LOG_D("i2c_readb: wait scl pin high "
                  "timeout at bit %d",
                  7 - i);

            return -SDK_E_TIMEOUT;
        }

        if (GET_SDA(ops))
            data |= 1;
        SCL_L(ops);
        i2c_delay2(ops);
    }

    return data;
}

static uint32_t i2c_send_bytes(sdk_swi2c_t *bus,
                               struct i2c_msg *msg)
{
    int32_t ret;
    uint32_t bytes = 0;
    const uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    uint16_t ignore_nack = msg->flags & I2C_FLAG_IGNORE_NACK;

    while (count > 0)
    {
        ret = i2c_writeb(bus, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0)))
        {
            count--;
            ptr++;
            bytes++;
        }
        else if (ret == 0)
        {
            LOG_D("send bytes: NACK.");

            return 0;
        }
        else
        {
            LOG_E("send bytes: error %d", ret);

            return ret;
        }
    }

    return bytes;
}

static sdk_err_t i2c_send_ack_or_nack(sdk_swi2c_t *bus, int ack)
{
    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;

    if (ack)
        SET_SDA(ops, 0);
    i2c_delay(ops);
    if (SCL_H(ops) < 0)
    {
        LOG_E("ACK or NACK timeout.");

        return -SDK_E_TIMEOUT;
    }
    SCL_L(ops);

    return SDK_OK;
}

static uint32_t i2c_recv_bytes(sdk_swi2c_t *bus,
                               struct i2c_msg *msg)
{
    int32_t val;
    int32_t bytes = 0; /* actual bytes */
    uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    const uint32_t flags = msg->flags;

    while (count > 0)
    {
        val = i2c_readb(bus);
        if (val >= 0)
        {
            *ptr = val;
            bytes++;
        }
        else
        {
            break;
        }

        ptr++;
        count--;

        LOG_D("recieve bytes: 0x%02x, %s",
              val, (flags & I2C_FLAG_READ_ACK) ? "(No ACK/NACK)" : (count ? "ACK" : "NACK"));

        if (!(flags & I2C_FLAG_READ_ACK))
        {
            val = i2c_send_ack_or_nack(bus, count);
            if (val < 0)
                return val;
        }
    }

    return bytes;
}

static int32_t i2c_send_address(sdk_swi2c_t *bus,
                                uint8_t addr,
                                int32_t retries)
{
    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;
    int32_t i;
    sdk_err_t ret = 0;

    for (i = 0; i <= retries; i++)
    {
        ret = i2c_writeb(bus, addr);
        if (ret == 1 || i == retries)
            break;
        LOG_D("send stop condition");
        i2c_stop(ops);
        i2c_delay2(ops);
        LOG_D("send start condition");
        i2c_start(ops);
    }

    return ret;
}

static sdk_err_t i2c_bit_send_address(sdk_swi2c_t *bus,
                                      struct i2c_msg *msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & I2C_FLAG_IGNORE_NACK;
    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;

    uint8_t addr1, addr2;
    int32_t retries;
    sdk_err_t ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & I2C_FLAG_ADDR_10BIT)
    {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;

        LOG_D("addr1: %d, addr2: %d", addr1, addr2);

        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            LOG_W("NACK: sending first addr");

            return -SDK_ERROR;
        }

        ret = i2c_writeb(bus, addr2);
        if ((ret != 1) && !ignore_nack)
        {
            LOG_W("NACK: sending second addr");

            return -SDK_ERROR;
        }
        if (flags & I2C_FLAG_RD)
        {
            LOG_D("send repeated start condition");
            i2c_restart(ops);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != 1) && !ignore_nack)
            {
                LOG_E("NACK: sending repeated addr");

                return -SDK_ERROR;
            }
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & I2C_FLAG_RD)
            addr1 |= 1;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
            return -SDK_ERROR;
    }

    return SDK_OK;
}

static uint32_t i2c_bit_xfer(sdk_swi2c_t *bus, struct i2c_msg msgs[], uint32_t num)
{
    struct i2c_msg *msg;
    struct sdk_swi2c_ops *ops = (struct sdk_swi2c_ops *)bus->ops;
    int32_t ret;
    uint32_t i;
    uint16_t ignore_nack;

    if (num == 0)
        return 0;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        ignore_nack = msg->flags & I2C_FLAG_IGNORE_NACK;
        if (!(msg->flags & I2C_FLAG_NO_START))
        {
            if (i)
            {
                i2c_restart(ops);
            }
            else
            {
                LOG_D("send start condition");
                i2c_start(ops);
            }
            ret = i2c_bit_send_address(bus, msg);
            if ((ret != SDK_OK) && !ignore_nack)
            {
                LOG_D("receive NACK from device addr 0x%02x msg %d",
                      msgs[i].addr, i);
                goto out;
            }
        }
        if (msg->flags & I2C_FLAG_RD)
        {
            ret = i2c_recv_bytes(bus, msg);
            if (ret >= 1)
            {
                LOG_D("read %d byte%s", ret, ret == 1 ? "" : "s");
            }
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -SDK_ERROR;
                goto out;
            }
        }
        else
        {
            ret = i2c_send_bytes(bus, msg);
            if (ret >= 1)
            {
                LOG_D("write %d byte%s", ret, ret == 1 ? "" : "s");
            }
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -SDK_ERROR;
                goto out;
            }
        }
    }
    ret = i;

out:
    if (!(msg->flags & I2C_FLAG_NO_STOP))
    {
        LOG_D("send stop condition");
        i2c_stop(ops);
    }

    return ret;
}

uint32_t sdk_swi2c_write(sdk_swi2c_t *bus, uint16_t addr, const uint8_t *buf, uint32_t count)
{
    uint32_t ret;
    struct i2c_msg msg;

    msg.addr = addr;
    msg.flags = I2C_FLAG_WR;
    msg.len = count;
    msg.buf = (uint8_t *)buf;

    ret = i2c_bit_xfer(bus, &msg, 1);

    return ret;
}

uint32_t sdk_swi2c_read(sdk_swi2c_t *bus, uint16_t addr, uint8_t *buf, uint32_t count)
{
    uint32_t ret;
    struct i2c_msg msg;

    msg.addr = addr;
    msg.flags = I2C_FLAG_RD;
    msg.len = count;
    msg.buf = buf;

    ret = i2c_bit_xfer(bus, &msg, 1);

    return ret;
}

sdk_err_t sdk_swi2c_open(sdk_swi2c_t *bus)
{
    sdk_err_t ret;
    ret = bus->ops->init();

    return ret;
}

sdk_err_t sdk_swi2c_close(sdk_swi2c_t *bus)
{
    sdk_err_t ret;
    ret = bus->ops->deinit();

    return ret;
}
