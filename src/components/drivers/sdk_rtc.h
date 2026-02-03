/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-19     rgw          first version
 */

#ifndef __SDK_RTC_H
#define __SDK_RTC_H

#include "sdk_def.h"
#include "sdk_os_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sdk_rtc sdk_rtc_t;

struct sdk_rtc_ops
{
    sdk_err_t (*open)(sdk_rtc_t *rtc);
    sdk_err_t (*close)(sdk_rtc_t *rtc);
    sdk_err_t (*control)(sdk_rtc_t *rtc, int32_t cmd, void *args);
};

struct _sdk_rtc
{
    bool is_opened;
    struct sdk_os_mutex lock;
    struct sdk_rtc_ops ops;
};

sdk_err_t sdk_rtc_open(sdk_rtc_t *rtc);
sdk_err_t sdk_rtc_close(sdk_rtc_t *rtc);
sdk_err_t sdk_rtc_control(sdk_rtc_t *rtc, int32_t cmd, void *args);
sdk_err_t sdk_rtc_set_date(sdk_rtc_t *rtc, uint32_t year, uint32_t month, uint32_t day);
sdk_err_t sdk_rtc_set_time(sdk_rtc_t *rtc, uint32_t hour, uint32_t minute, uint32_t second);
sdk_err_t sdk_rtc_set_timestamp(sdk_rtc_t *rtc, time_t timestamp);
sdk_err_t sdk_rtc_get_timestamp(sdk_rtc_t *rtc, time_t *timestamp);
void sdk_printtime(time_t t);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_RTC_H */
