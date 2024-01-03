/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-04     rgw          first version
 */

#include "aft_sdk.h"
#include "sdk_uart.h"
#include "sdk_common.h"

void sdk_uart_open(sdk_uart_t *uart, int32_t baudrate, int32_t data_bit, char parity, int32_t stop_bit)
{
    uart->ops.open(uart, baudrate, data_bit, parity, stop_bit);
    uart->is_opened = true;
}

void sdk_uart_close(sdk_uart_t *uart)
{
    uart->ops.close(uart);
    uart->is_opened = false;
}

int32_t sdk_uart_write(sdk_uart_t *uart, const uint8_t *data, int32_t len)
{
    for(int32_t i = 0; i < len; i++)
    {
        uart->ops.putc(uart, data[i]);
    }
    return len;
}

#if defined(SDK_UART_USING_DMA)
static void sdk_uart_rx_dma(sdk_uart_t *uart)
{
    uint32_t dma_cnt = 0;
    uart_rx_fifo_t *rx_fifo;

    rx_fifo = &uart->rx_fifo;

    sdk_uart_control(uart, SDK_CONTROL_UART_GET_DMA_CNT, &dma_cnt);
    
    if(rx_fifo->put_index != SDK_UART_RX_FIFO_LEN - dma_cnt)
    {
        rx_fifo->put_index = SDK_UART_RX_FIFO_LEN - dma_cnt;
        if (rx_fifo->put_index >= SDK_UART_RX_FIFO_LEN)
            rx_fifo->put_index = 0;

        /* if the next position is read index, discard this 'read char' */
        if (rx_fifo->put_index == rx_fifo->get_index)
        {
            rx_fifo->get_index += 1;
            rx_fifo->is_full = true;
            if (rx_fifo->get_index >= SDK_UART_RX_FIFO_LEN)
                rx_fifo->get_index = 0;
        }
    }
}

int32_t sdk_uart_read(sdk_uart_t *uart, uint8_t *data, int32_t len)
{
    int size;
    uart_rx_fifo_t *rx_fifo;

    size = len;

    rx_fifo = &uart->rx_fifo;

    while (len)
    {
        int ch;
        
        sdk_uart_rx_dma(uart);

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == false))
        {
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= SDK_UART_RX_FIFO_LEN)
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == true)
        {
            rx_fifo->is_full = false;
        }

        *data = ch & 0xff;
        data++;
        len--;
    }

    return size - len;
}

#else

int32_t sdk_uart_read(sdk_uart_t *uart, uint8_t *data, int32_t len)
{
    int size;
    uart_rx_fifo_t *rx_fifo;

    size = len;

    rx_fifo = &uart->rx_fifo;

    while (len)
    {
        int ch;

        sdk_hw_interrupt_disable();

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == false))
        {
            /* no data, enable interrupt and break out */
            sdk_hw_interrupt_enable();
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= SDK_UART_RX_FIFO_LEN)
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == true)
        {
            rx_fifo->is_full = false;
        }

        sdk_hw_interrupt_enable();

        *data = ch & 0xff;
        data++;
        len--;
    }

    return size - len;
}

#endif


int32_t sdk_uart_read_until(sdk_uart_t *uart, uint8_t *data, int32_t len, int32_t timeout_ms)
{
    uint32_t rl = 0;
    uint32_t sum = 0;
    int32_t timeout_cnt = timeout_ms;
    
    do
    {
        if (len - sum == 0)
            break;
        rl = sdk_uart_read(uart, data + sum, len - sum);
        sum += rl;
        if(sum == 0)
            break;
        if(rl == 0)
        {
            sdk_os_delay_ms(1);
            timeout_cnt--;
        }
        else
        {
            timeout_cnt = timeout_ms;
        }
    } while (timeout_cnt);

    return sum;
}

int32_t sdk_uart_control(sdk_uart_t *uart, int32_t cmd, void *args)
{
    return uart->ops.control(uart, cmd, args);
}

int sdk_hw_usart_init(sdk_uart_t *uart)
{
    uart->lock = sdk_os_mutex_create("uart1");

    return SDK_OK;
}

void sdk_uart_rx_isr(sdk_uart_t *uart)
{
    int ch = -1;
    uart_rx_fifo_t *rx_fifo;

    rx_fifo = &uart->rx_fifo;

    while (1)
    {
        ch = uart->ops.getc(uart);
        if (ch == -1)
            break;

        sdk_hw_interrupt_disable();
        if(uart->rx_callback != NULL)
            uart->rx_callback(ch);
        rx_fifo->buffer[rx_fifo->put_index] = ch;
        rx_fifo->put_index += 1;
        if (rx_fifo->put_index >= SDK_UART_RX_FIFO_LEN)
            rx_fifo->put_index = 0;

        /* if the next position is read index, discard this 'read char' */
        if (rx_fifo->put_index == rx_fifo->get_index)
        {
            rx_fifo->get_index += 1;
            rx_fifo->is_full = true;
            if (rx_fifo->get_index >= SDK_UART_RX_FIFO_LEN)
                rx_fifo->get_index = 0;
        }
        sdk_hw_interrupt_enable();
    }
}

void sdk_uart_rx_getc(sdk_uart_t *uart, int ch)
{
    uart_rx_fifo_t *rx_fifo;

    rx_fifo = &uart->rx_fifo;

    if (ch == -1)
        return;

    if (uart->rx_callback != NULL)
        uart->rx_callback(ch);
    rx_fifo->buffer[rx_fifo->put_index] = ch;
    rx_fifo->put_index += 1;
    if (rx_fifo->put_index >= SDK_UART_RX_FIFO_LEN)
        rx_fifo->put_index = 0;

    /* if the next position is read index, discard this 'read char' */
    if (rx_fifo->put_index == rx_fifo->get_index)
    {
        rx_fifo->get_index += 1;
        rx_fifo->is_full = true;
        if (rx_fifo->get_index >= SDK_UART_RX_FIFO_LEN)
            rx_fifo->get_index = 0;
    }
}

void sdk_uart_set_rx_callback(sdk_uart_t *uart, sdk_err_t (*rx_callback)(uint8_t ch))
{
    uart->rx_callback = rx_callback;
}

void sdk_uart_set_rx_idle_callback(sdk_uart_t *uart, sdk_err_t (*rx_idle_callback)(void))
{
    uart->rx_idle_callback = rx_idle_callback;
}

void sdk_uart_set_rx_rto_callback(sdk_uart_t *uart, sdk_err_t (*rx_rto_callback)(void))
{
    uart->rx_rto_callback = rx_rto_callback;
}
