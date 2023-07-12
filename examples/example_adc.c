/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-6-19      rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_adc.h"
#include "board.h"
#include "SEGGER_RTT.h"

#define DBG_TAG "ex.adc"
#define DBG_LVL DBG_LOG
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

void example_adc_init(void)
{
    sdk_adc_open(&adc0);
}

void example_adc_run(void)
{
    uint32_t value = 0;
    sdk_adc_read(&adc0, 11, &value);
    LOG_D("adc0: value = %d\n", value);
}

int main(void)
{
    board_init();
    example_adc_init();
    while(1)
    {
        example_adc_run();
        sdk_os_delay_ms(1000);
    }
}