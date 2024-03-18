/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-04     rgw          first version
 */

#ifndef __SDK_UART_H
#define __SDK_UART_H

#include "sdk_def.h"
#include "sdk_os_port.h"
#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_UART_RX_FIFO_LEN
#define SDK_UART_RX_FIFO_LEN    512
#endif

typedef struct _sdk_uart sdk_uart_t;
typedef struct _rx_fifo uart_rx_fifo_t;

struct _rx_fifo
{
    uint8_t buffer[SDK_UART_RX_FIFO_LEN];
    uint16_t put_index, get_index;
    bool is_full;
};



struct sdk_uart_ops
{
    int32_t (*open)(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit);
    int32_t (*close)(sdk_uart_t *uart);
    int32_t (*putc)(sdk_uart_t *uart, int32_t ch);
    int32_t (*getc)(sdk_uart_t *uart);
    int32_t (*control)(sdk_uart_t *uart, int32_t cmd, void *args);
};

struct sdk_uart_settings
{
    int32_t baudrate;
    int32_t data_bit;
    char parity;
    int32_t stop_bit;
};

struct _sdk_uart
{
    SDK_INSTANCE_TYPE instance;
    struct sdk_uart_settings settings;
    int32_t irq;
    int32_t irq_prio;
    bool is_opened;
    uart_rx_fifo_t rx_fifo;
    sdk_os_mutex_t lock;
    struct sdk_uart_ops ops;
    sdk_err_t (*rx_callback)(uint8_t ch);
    sdk_err_t (*rx_idle_callback)(void);
    sdk_err_t (*rx_rto_callback)(void);
};

void sdk_uart_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit);
void sdk_uart_close(sdk_uart_t *uart);
int32_t sdk_uart_write(sdk_uart_t *uart, const uint8_t *data, int32_t len);
int32_t sdk_uart_read(sdk_uart_t *uart, uint8_t *data, int32_t len);
int32_t sdk_uart_read_until(sdk_uart_t *uart, uint8_t *data, int32_t len, int32_t timeout_ms);
int32_t sdk_uart_control(sdk_uart_t *uart, int32_t cmd, void *args);
void sdk_uart_rx_isr(sdk_uart_t *uart);
void sdk_uart_rx_getc(sdk_uart_t *uart, int ch);
void sdk_uart_set_rx_callback(sdk_uart_t *uart, sdk_err_t (*rx_callback)(uint8_t ch));
void sdk_uart_set_rx_idle_callback(sdk_uart_t *uart, sdk_err_t (*rx_idle_callback)(void));
void sdk_uart_set_rx_rto_callback(sdk_uart_t *uart, sdk_err_t (*rx_rto_callback)(void));

#ifdef __cplusplus
}
#endif

#endif /* __SDK_UART_H */
