/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-12     rgw             first version
 */

#ifndef __SDK_OS_PORT_RTTHREAD_H
#define __SDK_OS_PORT_RTTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aft_sdk.h"
#include "rtthread.h"

/*===========================================================================*/
/* mutex type                                                                */
/*===========================================================================*/
struct sdk_os_mutex {
  struct rt_mutex mutex;
};


/*===========================================================================*/
/* semaphore type                                                            */
/*===========================================================================*/
struct sdk_os_semaphore
{
    struct rt_semaphore sem;
};
enum sdk_os_ipc_state
{
    UNINITIALIZED = 0,
    INITIALIZED = 1
} ;
/*===========================================================================*/
/* event type                                                                */
/*===========================================================================*/
struct sdk_os_event
{
    enum sdk_os_ipc_state state;
    struct rt_event event;
};

/*===========================================================================*/
/* mq type                                                                   */
/*===========================================================================*/
struct sdk_os_mq
{
    enum sdk_os_ipc_state state;
    struct rt_messagequeue mq;
};

#ifdef __cplusplus
}
#endif

#endif /* __SDK_OS_PORT_RTTHREAD_H */
