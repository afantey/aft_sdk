/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         {name}       description
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
typedef void* sdk_os_event_t;
typedef void* sdk_os_semaphore_t;
typedef void* sdk_os_mutex_t;

sdk_os_mutex_t sdk_os_mutex_create(const char* name);
void sdk_os_delay_ms(int delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_OS_PORT_H */
