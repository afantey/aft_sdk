/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */
 
#include "aft_sdk.h"
#include "sdk_board.h"

__WEAK void sdk_hw_console_output(const char *str)
{
}

__WEAK void sdk_hw_console_putc(const int ch)
{
}

__WEAK void sdk_hw_us_delay(uint32_t us)
{
    (void) us;
}

__WEAK void sdk_hw_interrupt_enable(void)
{
}

__WEAK void sdk_hw_interrupt_disable(void)
{
}

void sdk_hw_ms_delay(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        sdk_hw_us_delay(1000);
    }
}

__WEAK uint32_t sdk_hw_get_systick(void)
{
    return 0;
}

__WEAK void sdk_hw_system_reset(void)
{
}
