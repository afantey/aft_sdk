/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-12     rgw             first version
 */

#include "aft_sdk.h"

#include "rtthread.h"

// void sdk_os_init(void)
// {
// }

// void sdk_os_schedule(void)
// {
// }

// void sdk_os_task_startup(sdk_os_task_t task)
// {
// }

// sdk_os_task_t *sdk_os_task_create(const char *name,
//                                   void *params, size_t stack_size, int32_t priority)
// {
//     return NULL;
// }

// void sdk_os_task_delete(sdk_os_task_t task)
// {
// }

void sdk_os_delay_ms(int delay_ms)
{
    rt_thread_mdelay(delay_ms);
}

void sdk_os_enter_critical(void)
{
    rt_enter_critical();
}

void sdk_os_exit_critical(void)
{
    rt_exit_critical();
}

/*===========================================================================*/
/* Event                                                                     */
/*===========================================================================*/

sdk_err_t sdk_os_event_init(sdk_os_event_t event)
{
    static int m_event_id = 0;
    rt_event_t ep = &event->event;
    char name[8] = {0};
    snprintf(name, sizeof(name), "evt%d", m_event_id++);
    rt_event_init(ep, name, RT_IPC_FLAG_FIFO);
    return SDK_OK;
}

sdk_err_t sdk_os_event_send(sdk_os_event_t event, uint32_t set)
{
    rt_err_t result = RT_EOK;
    rt_event_t ep = &event->event;
    result = rt_event_send(ep, set);
    if (result != RT_EOK)
        return -SDK_ERROR;
    return SDK_OK;
}

sdk_err_t sdk_os_event_recv(sdk_os_event_t event, uint32_t set, uint8_t option, int32_t timeout, uint32_t *recved)
{
    if (set == 0)
        return -SDK_ERROR;

    rt_err_t result = RT_EOK;
    rt_event_t ep = &event->event;
    uint8_t rtoption = 0;

    /* check event set */
    if (option & SDK_EVENT_FLAG_AND)
    {
        rtoption |= RT_EVENT_FLAG_AND;
    }
    if (option & SDK_EVENT_FLAG_OR)
    {
        rtoption |= RT_EVENT_FLAG_OR;
    }
    if (option & SDK_EVENT_FLAG_CLEAR)
    {
        rtoption |= RT_EVENT_FLAG_CLEAR;
    }
    
    result = rt_event_recv(ep, set, rtoption, timeout, recved);
    if (result == -RT_ETIMEOUT)
    {
        return -SDK_E_TIMEOUT;
    }
    if (result != RT_EOK)
    {
        return -SDK_ERROR;
    }
    return SDK_OK;
}

/*===========================================================================*/
/* Semaphore                                                                 */
/*===========================================================================*/

sdk_err_t sdk_os_sem_init(sdk_os_sem_t sem, int32_t count)
{
    static int m_sem_id = 0;
    rt_sem_t sp = &sem->sem;
    char name[8] = {0};
    snprintf(name, sizeof(name), "sem%d", m_sem_id++);
    rt_sem_init(sp, name, count, RT_IPC_FLAG_FIFO);
    return SDK_OK;
}

sdk_err_t sdk_os_sem_take(sdk_os_sem_t sem, int32_t timeout)
{
    rt_err_t result = RT_EOK;
    rt_sem_t sp = &sem->sem;
    result = rt_sem_take(sp, timeout);
    if (result == -RT_ETIMEOUT)
    {
        return -SDK_E_TIMEOUT;
    }
    if (result != RT_EOK)
    {
        return -SDK_ERROR;
    }
    return SDK_OK;
}

sdk_err_t sdk_os_sem_release(sdk_os_sem_t sem)
{
    rt_err_t result = RT_EOK;
    rt_sem_t sp = &sem->sem;
    result = rt_sem_release(sp);
    if (result != RT_EOK)
    {
        return -SDK_ERROR;
    }
    return SDK_OK;
}

/*===========================================================================*/
/* mutex                                                                     */
/*===========================================================================*/

sdk_err_t sdk_os_mutex_init(sdk_os_mutex_t mutex)
{
    static int m_mutex_id = 0;
    rt_mutex_t mp = &mutex->mutex;
    char name[8] = {0};
    snprintf(name, sizeof(name), "mtx%d", m_mutex_id++);
    rt_mutex_init(mp, name, RT_IPC_FLAG_PRIO);
    return SDK_OK;
}

sdk_err_t sdk_os_mutex_take(sdk_os_mutex_t mutex, int32_t timeout_ms)
{
    rt_err_t result = RT_EOK;
    rt_mutex_t mp = &mutex->mutex;
    result = rt_mutex_take(mp, timeout_ms);
    if(result == -RT_ETIMEOUT)
    {
        return -SDK_E_TIMEOUT;
    }
    if(result != RT_EOK)
    {
        return -SDK_ERROR;
    }
    return SDK_OK;
}

int sdk_os_mutex_release(sdk_os_mutex_t mutex)
{
    rt_err_t result = RT_EOK;
    rt_mutex_t mp = &mutex->mutex;
    result = rt_mutex_release(mp);
    if(result != RT_EOK)
    {
        return -SDK_ERROR;
    }
    return SDK_OK;
}
