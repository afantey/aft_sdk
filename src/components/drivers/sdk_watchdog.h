/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-05-17     rgw             first version
 */

#ifndef  __SDK_WATCHDOG
#define  __SDK_WATCHDOG

#include "aft_sdk.h"
#include "sdk_os_port.h"

typedef struct sdk_watchdog_device sdk_watchdog_t;

struct sdk_watchdog_ops
{
    sdk_err_t (*open)(sdk_watchdog_t *wdt);
    sdk_err_t (*control)(sdk_watchdog_t *wdt, int cmd, void *arg);
};

struct sdk_watchdog_device
{
    bool is_opened;
    sdk_os_mutex_t lock;
    struct sdk_watchdog_ops ops;
};

sdk_err_t sdk_watchdog_open(sdk_watchdog_t *wdt);
sdk_err_t sdk_watchdog_close(sdk_watchdog_t *wdt);
sdk_err_t sdk_watchdog_control(sdk_watchdog_t *wdt, int cmd, void *args);

#endif /* __SDK_WATCHDOG */
