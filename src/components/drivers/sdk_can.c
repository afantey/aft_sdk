/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-04     rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_can.h"
#include "sdk_common.h"

sdk_err_t sdk_can_open(sdk_can_t *can)
{
    sdk_err_t ret = SDK_OK;

    if (can == NULL || can->ops.open == NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = can->ops.open(can);
    if(ret == SDK_OK)
    {
        can->is_opened = true;
    }
    return ret;
}

sdk_err_t sdk_can_close(sdk_can_t *can)
{
    sdk_err_t ret = SDK_OK;

    if (can == NULL || can->ops.close== NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = can->ops.close(can);
    if (ret == SDK_OK)
    {
        can->is_opened = false;
    }

    return ret;
}

/* read from message queue */
int32_t sdk_can_read(sdk_can_t *can, sdk_can_msg_t *msg)
{
    int size;
    can_rx_mq_t *rx_mq;

    rx_mq = &can->rx_mq;

    sdk_hw_interrupt_disable();

    if ((rx_mq->get_index == rx_mq->put_index) && (rx_mq->is_full == false))
    {
        sdk_hw_interrupt_enable();
        return 0;
    }

    *msg = rx_mq->msg_queue[rx_mq->get_index];
    rx_mq->get_index += 1;
    if (rx_mq->get_index >= SDK_CAN_RX_MQ_LEN)
        rx_mq->get_index = 0;

    if (rx_mq->is_full == true)
    {
        rx_mq->is_full = false;
    }

    sdk_hw_interrupt_enable();

    return 1;
}

sdk_err_t sdk_can_write(sdk_can_t *can, sdk_can_msg_t *msg)
{
    sdk_err_t ret = SDK_OK;

    if (can == NULL || can->ops.write == NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = can->ops.write(can, msg);
    return ret;
}

sdk_err_t sdk_can_control(sdk_can_t *can, int32_t cmd, void *args)
{
    sdk_err_t ret = SDK_OK;

    if (can == NULL || can->ops.control == NULL)
    {
        return -SDK_E_INVALID;
    }
    ret = can->ops.control(can, cmd, args);
    return ret;
}

void sdk_can_rx_isr(sdk_can_t *can)
{
    sdk_can_msg_t msg = {0};
    can_rx_mq_t *rx_mq;
    bool is_recved = false;

    rx_mq = &can->rx_mq;

    is_recved = can->ops.read(can, &msg);
    if (is_recved == false)
        return;

    sdk_hw_interrupt_disable();

    rx_mq->msg_queue[rx_mq->put_index] = msg;
    rx_mq->put_index += 1;
    if (rx_mq->put_index >= SDK_CAN_RX_MQ_LEN)
        rx_mq->put_index = 0;

    /* if the next position is read index, discard this 'read char' */
    if (rx_mq->put_index == rx_mq->get_index)
    {
        rx_mq->get_index += 1;
        rx_mq->is_full = true;
        if (rx_mq->get_index >= SDK_CAN_RX_MQ_LEN)
            rx_mq->get_index = 0;
    }
    sdk_hw_interrupt_enable();
}
