/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_uart.h"
#include "board.h"

void uart1_init(void)
{
    sdk_uart_open(&uart1, 115200, 8, 'N', 1);
    sdk_uart_control(&uart1, SDK_CONTROL_UART_ENABLE_INT, NULL);
    sdk_uart_write(&uart1, (uint8_t *)"Hello sdk\n", strlen("Hello sdk\n"));
}

void uart1_run(void)
{
    uint8_t buf[20] = {0};
    int rl = 0;
    rl = sdk_uart_read(&uart1, buf, 20);
    if (rl > 0)
    {
        sdk_uart_write(&uart1, buf, rl);
    }
}

int main(void)
{
    board_init();
    uart1_init();
    while(1)
    {
        uart1_run();
        //sdk_os_delay_ms(1);
    }
}