/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-12     rgw             first version
 */

#include "sdk_common.h"
#include "sdk_def.h"
#include "sdk_os_port.h"

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

sdk_err_t sdk_os_event_create(sdk_os_event_t event)
{
    event->state = EVENT_STATE_START;
    event->set = 0;
    return SDK_OK;
}

void sdk_os_event_delete(sdk_os_event_t event)
{
}

sdk_err_t sdk_os_event_send(sdk_os_event_t event, uint32_t set)
{
    if (set == 0)
        return -SDK_ERROR;
    event->set |= set;
    return SDK_OK;
}

sdk_err_t sdk_os_event_recv(sdk_os_event_t event, uint32_t set, uint8_t option, int32_t timeout, uint32_t *recved)
{
    if (set == 0)
        return -SDK_ERROR;
    sdk_err_t status = -SDK_ERROR;

    /* check event set */
    if (option & SDK_EVENT_FLAG_AND)
    {
        if ((event->set & set) == set)
            status = SDK_OK;
    }
    else if (option & SDK_EVENT_FLAG_OR)
    {
        if (event->set & set)
            status = SDK_OK;
    }
    else
    {
        /* either SDK_EVENT_FLAG_AND or SDK_EVENT_FLAG_OR should be set */
    }

    if (status == SDK_OK)
    {
        event->state = EVENT_STATE_START;
        /* set received event */
        if (recved)
            *recved = (event->set & set);

        /* received event */
        if (option & SDK_EVENT_FLAG_CLEAR)
            event->set &= ~set;
    }
    else if (timeout == 0)
    {
        status = -SDK_E_TIMEOUT;
    }
    else
    {
        switch (event->state)
        {
        case EVENT_STATE_START:
            swtimer_set(&event->timer, timeout);
            event->state = EVENT_STATE_WAITING;
            status = -SDK_E_BUSY;
            break;
        case EVENT_STATE_WAITING:
            if(swtimer_expired(&event->timer))
            {
                event->state = EVENT_STATE_TIMEOUT;
            }
            status = -SDK_E_BUSY;
            break;
        case EVENT_STATE_TIMEOUT:
            event->state = EVENT_STATE_START;
            status = -SDK_E_TIMEOUT;
        default:
            break;
        }
    }
    return status;
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

sdk_os_mutex_t sdk_os_mutex_create(const char *name)
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
