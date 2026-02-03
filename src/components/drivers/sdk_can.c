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
    if(can->state == CAN_UNINIT)
    {
        ret = can->ops.open(can);
        if (ret == SDK_OK)
        {
            can->state = CAN_READY;
        }
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
        can->state = CAN_UNINIT;
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


static const struct sdk_canid_table *get_canid_table_obj(sdk_can_t *can, uint32_t canid)
{
    const struct sdk_canid_table *canid_table = NULL;

    if (can->canid_list.next == NULL)
    {
        return NULL;
    }

    canid_table = can->canid_list.next;

    while(canid_table != NULL)
    {
        if (canid_table->canid == canid)
        {
            return canid_table;
        }
        else
        {
            canid_table = canid_table->next;
            continue;
        }
    }

    return NULL;
}

static int sdk_can_msg_parse(sdk_can_t *can, sdk_can_msg_t *msg)
{
    int ret = 0;
    const struct sdk_canid_table *canid_table = NULL;
    canid_table = get_canid_table_obj(can, msg->canid);
    if(canid_table != NULL && canid_table->msg_handler != NULL)
    {
        ret = canid_table->msg_handler(can->can_dev, msg);
    }

    return ret;
}

int sdk_can_recv_poll(sdk_can_t *can)
{
    sdk_can_msg_t msg = {0};
    int rl = sdk_can_read(can, &msg);
    if(rl > 0)
    {
        sdk_can_msg_parse(can, &msg);
    }
    return rl;
}

void sdk_can_register(sdk_can_t *can, void *can_dev, struct sdk_canid_table *canid_table, size_t table_sz)
{
    can->can_dev = can_dev;

    struct sdk_canid_table *current = &can->canid_list;
    struct sdk_canid_table *check = NULL;

    // 遍历canid_table数组
    for (int i = 0; i < table_sz; i++)
    {
        // 检查链表中是否已经存在相同的节点
        check = &can->canid_list;
        while (check != NULL)
        {
            if (check == &canid_table[i])
            {
                // 如果找到相同的节点，跳过这个节点
                break;
            }
            check = check->next;
        }

        // 如果没有找到相同的节点，将canid_table[i]添加到链表的末尾
        if (check == NULL)
        {
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = &canid_table[i];
        }
    }
}
