/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-04     rgw          first version
 */

#ifndef __SDK_CAN_H
#define __SDK_CAN_H

#include "sdk_def.h"
#include "sdk_os_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_CAN_RX_MQ_LEN
#define SDK_CAN_RX_MQ_LEN    32
#endif

#define SDK_CAN_IDE_STANDARD 0
#define SDK_CAN_IDE_EXTENDED 1

#define SDK_CAN_RTR_DATA     0
#define SDK_CAN_RTR_REMOTE   1

typedef struct _sdk_can sdk_can_t;
typedef struct _can_rx_mq can_rx_mq_t;

struct sdk_can_msg
{
    uint32_t canid : 29;
    uint32_t ide : 1;
    uint32_t rtr : 1;
    uint32_t rsv1 : 1;
    uint32_t dlc : 4;
    uint32_t reserved : 28;
    uint8_t data[8];
};
typedef struct sdk_can_msg sdk_can_msg_t;

struct _can_rx_mq
{
    sdk_can_msg_t msg_queue[SDK_CAN_RX_MQ_LEN];
    uint16_t put_index, get_index;
    bool is_full;
};

struct sdk_can_ops
{
    sdk_err_t (*open)(sdk_can_t *can);
    sdk_err_t (*close)(sdk_can_t *can);
    int32_t (*read)(sdk_can_t *can, sdk_can_msg_t *msg);
    sdk_err_t (*write)(sdk_can_t *can, sdk_can_msg_t *msg);
    sdk_err_t (*control)(sdk_can_t *can, int32_t cmd, void *args);
};

enum canstate
{
    CAN_UNINIT   = 0, /**< Not initialized.           */
    CAN_STOP     = 1, /**< Stopped.                   */
    CAN_STARTING = 2, /**< Starting.                  */
    CAN_STOPPING = 3, /**< Stopping.                  */
    CAN_READY    = 4, /**< Ready.                     */
    CAN_SLEEP    = 5 /**< Sleep state.               */
};

struct _sdk_can
{
    SDK_INSTANCE_TYPE instance;
    uint32_t clock;
    int32_t irq;
    int32_t irq_prio;
    enum canstate state;
    int32_t can_fifo_num;
    int32_t can_interrupt;
    can_rx_mq_t rx_mq;
    struct sdk_os_mutex lock;
    struct sdk_can_ops ops;
};

sdk_err_t sdk_can_open(sdk_can_t *can);
sdk_err_t sdk_can_close(sdk_can_t *can);
sdk_err_t sdk_can_write(sdk_can_t *can, sdk_can_msg_t *msg);
int32_t sdk_can_read(sdk_can_t *can, sdk_can_msg_t *msg);
sdk_err_t sdk_can_control(sdk_can_t *can, int32_t cmd, void *args);
void sdk_can_rx_isr(sdk_can_t *can);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_CAN_H */
