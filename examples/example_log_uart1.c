/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_uart.h"
#include "sdk_board.h"

#define DBG_TAG "ex.log"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

void sdk_hw_console_output(const char *str)
{
    sdk_uart_write(&uart_console, (uint8_t *)str, strlen(str));
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART1, (uint8_t)ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
        ;
    return ch;
}

void log_uart_init(void)
{
    sdk_uart_open(&uart_console, 115200, 8, 'N', 1);
    sdk_uart_control(&uart_console, SDK_CONTROL_UART_ENABLE_INT, NULL);
    sdk_uart_write(&uart_console, (uint8_t *)"Hello sdk\n", strlen("Hello sdk\n"));
}

uint8_t test[10] = {1,2,3,4,5,6,7,8,9,0};
void log_uart_run(void)
{
    LOG_D("debug log running\n");
    LOG_I("info log running\n");
    LOG_W("warning log running\n");
    LOG_E("error log running\n");
    LOG_HEX(8, test, 10);
}

int main(void)
{
    nvic_vector_table_set(0x08000000 , 0x2000);
    board_init();
    log_uart_init();
    while(1)
    {
        log_uart_run();
        sdk_os_delay_ms(1000);
    }
}