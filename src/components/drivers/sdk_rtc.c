/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-19     rgw          first version
 */

#include "sdk_rtc.h"
#include "sdk_log.h"

sdk_err_t sdk_rtc_open(sdk_rtc_t *rtc)
{
    sdk_err_t ret = SDK_OK;
    if(rtc->is_opened != true)
    {
        rtc->is_opened = true;
        ret = rtc->ops.open(rtc);
    }
    else
    {
        ret = SDK_OK;
    }

    return ret;
}

sdk_err_t sdk_rtc_close(sdk_rtc_t *rtc)
{
    rtc->is_opened = false;
    return rtc->ops.close(rtc);
}

sdk_err_t sdk_rtc_control(sdk_rtc_t *rtc, int32_t cmd, void *args)
{
    return rtc->ops.control(rtc, cmd, args);
}


/**
 * Set system date(time not modify, local timezone).
 *
 * @param uint32_t year  e.g: 2012.
 * @param uint32_t month e.g: 12 (1~12).
 * @param uint32_t day   e.g: 31.
 *
 * @return sdk_err_t if set success, return SDK_OK.
 */
sdk_err_t sdk_rtc_set_date(sdk_rtc_t *rtc, uint32_t year, uint32_t month, uint32_t day)
{
    time_t now, old_timestamp = 0;
    struct tm tm_new = {0};
    sdk_err_t ret = -SDK_ERROR;

    /* get current time */
    ret = sdk_rtc_control(rtc, SDK_CONTROL_RTC_GET_TIME, &old_timestamp);
    if (ret != SDK_OK)
    {
        return ret;
    }

    /* converts calendar time into local time. */
    localtime_r(&old_timestamp, &tm_new);
    

    /* update date. */
    tm_new.tm_year = year - 1900;
    tm_new.tm_mon  = month - 1; /* tm_mon: 0~11 */
    tm_new.tm_mday = day;

    /* converts the local time into the calendar time. */
    now = mktime(&tm_new);

    /* update to RTC device. */
    ret = sdk_rtc_control(rtc, SDK_CONTROL_RTC_SET_TIME, &now);
    return ret;
}

/**
 * Set system time(date not modify, local timezone).
 *
 * @param uint32_t hour   e.g: 0~23.
 * @param uint32_t minute e.g: 0~59.
 * @param uint32_t second e.g: 0~59.
 *
 * @return sdk_err_t if set success, return SDK_OK.
 */
sdk_err_t sdk_rtc_set_time(sdk_rtc_t *rtc, uint32_t hour, uint32_t minute, uint32_t second)
{
    time_t now, old_timestamp = 0;
    struct tm tm_new = {0};
    sdk_err_t ret = -SDK_ERROR;

    /* get current time */
    ret = sdk_rtc_control(rtc, SDK_CONTROL_RTC_GET_TIME, &old_timestamp);
    if (ret != SDK_OK)
    {
        return ret;
    }

    /* converts calendar time into local time. */
    localtime_r(&old_timestamp, &tm_new);

    /* update time. */
    tm_new.tm_hour = hour;
    tm_new.tm_min  = minute;
    tm_new.tm_sec  = second;

    /* converts the local time into the calendar time. */
    now = mktime(&tm_new);

    /* update to RTC device. */
    ret = sdk_rtc_control(rtc, SDK_CONTROL_RTC_SET_TIME, &now);
    return ret;
}

/**
 * Set timestamp(UTC).
 *
 * @param time_t timestamp
 *
 * @return sdk_err_t if set success, return SDK_OK.
 */
sdk_err_t sdk_rtc_set_timestamp(sdk_rtc_t *rtc, time_t timestamp)
{
    /* update to RTC device. */
    return sdk_rtc_control(rtc, SDK_CONTROL_RTC_SET_TIME, &timestamp);
}

/**
 * Get timestamp(UTC).
 *
 * @param time_t* timestamp
 *
 * @return sdk_err_t if set success, return SDK_OK.
 */
sdk_err_t sdk_rtc_get_timestamp(sdk_rtc_t *rtc, time_t *timestamp)
{
    /* Get timestamp from RTC device. */
    return sdk_rtc_control(rtc, SDK_CONTROL_RTC_GET_TIME, timestamp);
}

void sdk_printtime(time_t t)
{
    struct tm p_tm;
    
    localtime_r(&t, &p_tm);

    sdk_printf("time: %04d-%02d-%02d %02d:%02d:%02d\n", p_tm.tm_year + 1900, p_tm.tm_mon + 1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec);
}
