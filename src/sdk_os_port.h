/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-12     rgw             first version
 */

#ifndef __SDK_OS_PORT_H
#define __SDK_OS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aft_sdk.h"

#if defined(SDK_USE_NO_RTOS) || defined(SDK_USE_ZEPHYR)
    #include "sdk_os_port_none.h"
#endif

#ifdef SDK_USE_RTTHREAD
    #include "sdk_os_port_rtthread.h"
#endif

#define SDK_WAITING_FOREVER              -1              /**< Block forever until get resource. */
#define SDK_WAITING_NO                   0               /**< Non-block. */

void sdk_os_delay_ms(int delay_ms);
void sdk_os_enter_critical(void);
void sdk_os_exit_critical(void);


/*===========================================================================*/
/* semaphore                                                                 */
/*===========================================================================*/
typedef struct sdk_os_semaphore *sdk_os_sem_t;

sdk_err_t sdk_os_sem_init(sdk_os_sem_t sem, int32_t count);
sdk_err_t sdk_os_sem_delete(sdk_os_sem_t sem);
sdk_err_t sdk_os_sem_take(sdk_os_sem_t sem, int32_t timeout);
sdk_err_t sdk_os_sem_release(sdk_os_sem_t sem);

/*===========================================================================*/
/* event                                                                     */
/*===========================================================================*/
typedef struct sdk_os_event *sdk_os_event_t;

#define SDK_EVENT_FLAG_AND               0x01            /**< logic and */
#define SDK_EVENT_FLAG_OR                0x02            /**< logic or */
#define SDK_EVENT_FLAG_CLEAR             0x04            /**< clear flag */

sdk_err_t sdk_os_event_init(sdk_os_event_t event);
sdk_err_t sdk_os_event_delete(sdk_os_event_t event);
sdk_err_t sdk_os_event_send(sdk_os_event_t event, uint32_t set);
sdk_err_t sdk_os_event_recv(sdk_os_event_t event, uint32_t set, uint8_t option, int32_t timeout, uint32_t *recved);

/*===========================================================================*/
/* mutex                                                                     */
/*===========================================================================*/
typedef struct sdk_os_mutex *sdk_os_mutex_t;
sdk_err_t sdk_os_mutex_init(sdk_os_mutex_t mutex);
sdk_err_t sdk_os_mutex_take(sdk_os_mutex_t mutex, int32_t timeout_ms);
int sdk_os_mutex_release(sdk_os_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_OS_PORT_H */
