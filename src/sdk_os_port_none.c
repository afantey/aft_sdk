/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "sdk_def.h"
#include "sdk_os_port.h"
#include "sdk_common.h"

void sdk_os_init(void)
{
   
}

void sdk_os_schedule(void)
{

}

void sdk_os_task_startup(sdk_os_task_t task)
{

}

sdk_os_task_t *sdk_os_task_create(const char *name,
   void *params, size_t stack_size, int32_t priority)
{
   return NULL;
}

void sdk_os_task_delete(sdk_os_task_t task)
{
}

void sdk_os_delay_ms(int delay_ms)
{
    sdk_hw_ms_delay(delay_ms);
}

bool sdk_os_event_create(sdk_os_event_t event)
{
   return true;
}

void sdk_os_event_delete(sdk_os_event_t event)
{
}

void sdk_os_event_send(sdk_os_event_t event)
{
}

bool sdk_os_event_recv(sdk_os_event_t event, int timeout)
{
   return true;
}

bool sdk_os_semaphore_create(sdk_os_semaphore_t semaphore, int32_t count)
{
   return true;
}

void sdk_os_semaphore_delete(sdk_os_semaphore_t semaphore)
{
}

bool sdk_os_semaphore_take(sdk_os_semaphore_t semaphore, int32_t timeout)
{
   return true;
}

void sdk_os_semaphore_release(sdk_os_semaphore_t semaphore)
{
}

sdk_os_mutex_t sdk_os_mutex_create(const char* name)
{
    return NULL;
}

void sdk_os_mutex_delete(sdk_os_mutex_t mutex)
{
}

int sdk_os_mutex_take(sdk_os_mutex_t mutex, int32_t timeout)
{
    return 1;
}

int sdk_os_mutex_release(sdk_os_mutex_t mutex)
{
    return 1;
}

void sdk_os_enter_critical(void)
{

}

void sdk_os_exit_critical(void)
{

}
