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
    #include "sdk_os_port.h"
#endif

typedef void* sdk_os_task_t;
typedef void* sdk_os_semaphore_t;
typedef void* sdk_os_mutex_t;



// ----------------------------------------------------------------------------
// EVENT
// ----------------------------------------------------------------------------
#define SDK_EVENT_FLAG_AND               0x01            /**< logic and */
#define SDK_EVENT_FLAG_OR                0x02            /**< logic or */
#define SDK_EVENT_FLAG_CLEAR             0x04            /**< clear flag */

enum sdk_os_event_state
{
    EVENT_STATE_START,
    EVENT_STATE_WAITING,
    EVENT_STATE_TIMEOUT,
};

struct sdk_os_event
{
    enum sdk_os_event_state state;
    struct swtimer timer;
    uint32_t set;
};
typedef struct sdk_os_event *sdk_os_event_t;

sdk_err_t sdk_os_event_create(sdk_os_event_t event);
void sdk_os_event_delete(sdk_os_event_t event);
sdk_err_t sdk_os_event_send(sdk_os_event_t event, uint32_t set);
sdk_err_t sdk_os_event_recv(sdk_os_event_t event, uint32_t set, uint8_t option, int32_t timeout, uint32_t *recved);

// ----------------------------------------------------------------------------
// MUTEX
// ----------------------------------------------------------------------------
sdk_os_mutex_t sdk_os_mutex_create(const char* name);
void sdk_os_delay_ms(int delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_OS_PORT_H */
