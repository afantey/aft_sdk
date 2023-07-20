/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */
#include "aft_sdk.h"
#include "sdk_uart.h"
#include "sdk_board.h"
#include "SEGGER_RTT.h"

#define DBG_TAG "ex.rtc"
#define DBG_LVL DBG_ERROR
#include "sdk_log.h"

void sdk_hw_console_output(const char *str)
{
    SEGGER_RTT_WriteString(0, str);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    SEGGER_RTT_PutChar(0, (uint8_t)ch);
    return ch;
}

void example_rtc_init(void)
{
    time_t t = 0;
    sdk_rtc_open(&rtc);
    sdk_rtc_control(&rtc, SDK_CONTROL_RTC_GET_TIME, &t);
    sdk_printtime(t);
    t = 1672212552;
    sdk_rtc_control(&rtc, SDK_CONTROL_RTC_SET_TIME, &t);
    sdk_rtc_control(&rtc, SDK_CONTROL_RTC_GET_TIME, &t);
    sdk_printtime(t);
}

void example_rtc_run(void)
{
    time_t t = 0;
    sdk_rtc_control(&rtc, SDK_CONTROL_RTC_GET_TIME, &t);
    sdk_printtime(t);
    t = time(NULL);
    printf("time(NULL) ");
    sdk_printtime(t);
}

int main(void)
{
    board_init();
    example_rtc_init();

    while(1)
    {
        example_rtc_run();
        printf("clock = %d\n",clock());
        sdk_os_delay_ms(1000);
    }
}
