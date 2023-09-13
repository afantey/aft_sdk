/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-05-17     rgw             first version
 */

#include "sdk_watchdog.h"

#define DBG_TAG "bsp.wdt"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

sdk_err_t sdk_watchdog_open(sdk_watchdog_t *wdt)
{
    if (wdt->ops.open)
    {
        return (wdt->ops.open(wdt));
    }

    return (-SDK_ERROR);
}

sdk_err_t sdk_watchdog_close(sdk_watchdog_t *wdt)
{
    if (wdt->ops.control(wdt, SDK_CONTROL_WDT_STOP, NULL) != SDK_OK)
    {
        LOG_W(" This watchdog can not be stoped\n");

        return (-SDK_ERROR);
    }

    return (SDK_OK);
}

sdk_err_t sdk_watchdog_control(sdk_watchdog_t *wdt, int cmd, void *args)
{
    return (wdt->ops.control(wdt, cmd, args));
}
