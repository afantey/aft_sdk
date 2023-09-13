/**
 * Change Logs:
 * Date           Author          Notes
 * 2022-12-28     rgw             first version
 */

#include "aft_sdk.h"
#include "sdk_rtc.h"
#include "sdk_board.h"

extern sdk_rtc_t rtc;
time_t time (time_t *_timer)
{
    struct tm *ts;
    time_t time_dat;
    int ret;
    if(_timer != NULL)
    {
        ret = sdk_rtc_control(&rtc, SDK_CONTROL_RTC_SET_TIME, _timer);
        if(ret != SDK_OK)
        {
            time_dat = 0;
        }
        else
        {
            time_dat = *_timer;
        }
    }
    else
    {
        ret = sdk_rtc_control(&rtc, SDK_CONTROL_RTC_GET_TIME, &time_dat); 
    }
    return time_dat;
}

clock_t clock(void)
{
    return (clock_t)sdk_hw_get_systick();
}
