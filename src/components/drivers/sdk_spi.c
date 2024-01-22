/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-12-10     rgw             first version
 */

#include "aft_sdk.h"
#include "sdk_spi.h"


extern sdk_err_t sdk_spi_bus_device_init(struct sdk_spi_bus *bus, const char *name);
extern sdk_err_t sdk_spidev_device_init(struct sdk_spi_device *dev, const char *name);

sdk_err_t sdk_spi_bus_register(struct sdk_spi_bus *bus,
                               const char *name,
                               const struct sdk_spi_ops *ops)
{
    sdk_err_t result;

    result = sdk_spi_bus_device_init(bus, name);
    if (result != SDK_OK)
        return result;

    /* initialize mutex lock */
    // rt_mutex_init(&(bus->lock), name, RT_IPC_FLAG_PRIO);
    /* set ops */
    bus->ops = ops;
    /* initialize owner */
    bus->owner = NULL;
    /* set bus mode */
    bus->mode = RT_SPI_BUS_MODE_SPI;

    return SDK_OK;
}

sdk_err_t sdk_spi_configure(struct sdk_spi_device *device,
                            struct sdk_spi_configuration *cfg)
{
    sdk_err_t result;

    SDK_ASSERT(device != NULL);

    /* set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & RT_SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (device->bus != NULL)
    {
        result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
        if (result == SDK_OK)
        {
            if (device->bus->owner == device)
            {
                device->bus->ops->configure(device, &device->config);
            }

            /* release lock */
            rt_mutex_release(&(device->bus->lock));
        }
    }

    return SDK_OK;
}

sdk_err_t sdk_spi_send_then_send(struct sdk_spi_device *device,
                               const void           *send_buf1,
                               rt_size_t             send_length1,
                               const void           *send_buf2,
                               rt_size_t             send_length2)
{
    sdk_err_t result;
    struct sdk_spi_message message;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == SDK_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == SDK_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -SDK_ERROR;
                goto __exit;
            }
        }

        /* send data1 */
        message.send_buf   = send_buf1;
        message.recv_buf   = NULL;
        message.length     = send_length1;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -SDK_ERROR;
            goto __exit;
        }

        /* send data2 */
        message.send_buf   = send_buf2;
        message.recv_buf   = NULL;
        message.length     = send_length2;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -SDK_ERROR;
            goto __exit;
        }

        result = SDK_OK;
    }
    else
    {
        return -SDK_ERROR;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}

sdk_err_t sdk_spi_send_then_recv(struct sdk_spi_device *device,
                               const void           *send_buf,
                               rt_size_t             send_length,
                               void                 *recv_buf,
                               rt_size_t             recv_length)
{
    sdk_err_t result;
    struct sdk_spi_message message;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == SDK_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == SDK_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -SDK_ERROR;
                goto __exit;
            }
        }

        /* send data */
        message.send_buf   = send_buf;
        message.recv_buf   = NULL;
        message.length     = send_length;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -SDK_ERROR;
            goto __exit;
        }

        /* recv data */
        message.send_buf   = NULL;
        message.recv_buf   = recv_buf;
        message.length     = recv_length;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -SDK_ERROR;
            goto __exit;
        }

        result = SDK_OK;
    }
    else
    {
        return -SDK_ERROR;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}

rt_size_t sdk_spi_transfer(struct sdk_spi_device *device,
                          const void           *send_buf,
                          void                 *recv_buf,
                          rt_size_t             length)
{
    sdk_err_t result;
    struct sdk_spi_message message;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result == SDK_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == SDK_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                rt_set_errno(-SDK_ERROR);
                result = 0;
                goto __exit;
            }
        }

        /* initial message */
        message.send_buf   = send_buf;
        message.recv_buf   = recv_buf;
        message.length     = length;
        message.cs_take    = 1;
        message.cs_release = 1;
        message.next       = NULL;

        /* transfer message */
        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            rt_set_errno(-SDK_ERROR);
            goto __exit;
        }
    }
    else
    {
        rt_set_errno(-SDK_ERROR);

        return 0;
    }

__exit:
    rt_mutex_release(&(device->bus->lock));

    return result;
}

struct sdk_spi_message *sdk_spi_transfer_message(struct sdk_spi_device  *device,
                                               struct sdk_spi_message *message)
{
    sdk_err_t result;
    struct sdk_spi_message *index;

    SDK_ASSERT(device != NULL);

    /* get first message */
    index = message;
    if (index == NULL)
        return index;

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result != SDK_OK)
    {
        rt_set_errno(-SDK_E_BUSY);

        return index;
    }

    /* reset errno */
    rt_set_errno(SDK_OK);

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == SDK_OK)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            rt_set_errno(-SDK_ERROR);
            goto __exit;
        }
    }

    /* transmit each SPI message */
    while (index != NULL)
    {
        /* transmit SPI message */
        result = device->bus->ops->xfer(device, index);
        if (result == 0)
        {
            rt_set_errno(-SDK_ERROR);
            break;
        }

        index = index->next;
    }

__exit:
    /* release bus lock */
    rt_mutex_release(&(device->bus->lock));

    return index;
}

sdk_err_t sdk_spi_take_bus(struct sdk_spi_device *device)
{
    sdk_err_t result = SDK_OK;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
    if (result != SDK_OK)
    {
        rt_set_errno(-SDK_E_BUSY);

        return -SDK_E_BUSY;
    }

    /* reset errno */
    rt_set_errno(SDK_OK);

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == SDK_OK)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            rt_set_errno(-SDK_ERROR);
            /* release lock */
            rt_mutex_release(&(device->bus->lock));

            return -SDK_ERROR;
        }
    }

    return result;
}

sdk_err_t sdk_spi_release_bus(struct sdk_spi_device *device)
{
    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);
    SDK_ASSERT(device->bus->owner == device);

    /* release lock */
    rt_mutex_release(&(device->bus->lock));

    return SDK_OK;
}

sdk_err_t sdk_spi_take(struct sdk_spi_device *device)
{
    sdk_err_t result;
    struct sdk_spi_message message;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}

sdk_err_t sdk_spi_release(struct sdk_spi_device *device)
{
    sdk_err_t result;
    struct sdk_spi_message message;

    SDK_ASSERT(device != NULL);
    SDK_ASSERT(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}
