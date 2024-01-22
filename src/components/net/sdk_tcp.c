/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-23     rgw             first version
 */

#include "sdk_tcp.h"
#include "aft_sdk.h"
#include "sdk_board.h"

enum sdk_tcp_state sdk_tcp_fsm_poll(struct sdk_tcp *tcp)
{
    switch (tcp->state)
    {
    case SDK_TCP_STATE_IDLE:
    {
        sdk_err_t evt_err = 0;
        evt_err = sdk_os_event_recv(&tcp->event, SDK_TCP_EVENT_OPEN, SDK_EVENT_FLAG_OR | SDK_EVENT_FLAG_CLEAR, 0, NULL);
        if (evt_err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_OPEN;
        }
        break;
    }
    case SDK_TCP_STATE_OPEN:
    {
        sdk_err_t err = tcp->ops.open(tcp);
        if(err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_CONNECT;
        }
        else if(err == -SDK_E_TIMEOUT)
        {
            tcp->state = SDK_TCP_STATE_TIMEOUT;
        }
        break;
    }
    case SDK_TCP_STATE_CONNECT:
    {
        sdk_err_t err = tcp->ops.connect(tcp, tcp->ip, tcp->port);
        if(err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_RECV;
        }
        else if(err == -SDK_E_TIMEOUT)
        {
            tcp->state = SDK_TCP_STATE_TIMEOUT;
        }
        break;
    }
    case SDK_TCP_STATE_CLOSE:
    {
        sdk_err_t err = tcp->ops.close(tcp);
        if(err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_IDLE;
        }
        else if(err == -SDK_E_TIMEOUT)
        {
            tcp->state = SDK_TCP_STATE_TIMEOUT;
        }
        break;
    }
    case SDK_TCP_STATE_RECV:
    {
        sdk_err_t evt_err = SDK_OK;
        // send event
        evt_err = sdk_os_event_recv(&tcp->event, SDK_TCP_EVENT_SEND, SDK_EVENT_FLAG_OR | SDK_EVENT_FLAG_CLEAR, 0, NULL);
        if (evt_err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_SEND;
        }
        // close event 
        evt_err = sdk_os_event_recv(&tcp->event, SDK_TCP_EVENT_CLOSE, SDK_EVENT_FLAG_OR | SDK_EVENT_FLAG_CLEAR, 0, NULL);
        if (evt_err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_CLOSE;
        }
        // recv


        break;
    }
    case SDK_TCP_STATE_SEND:
    {
        sdk_err_t err = tcp->ops.send(tcp, tcp->send_buf, tcp->send_len);
        if(err == SDK_OK)
        {
            tcp->state = SDK_TCP_STATE_RECV;
        }
        else if(err == -SDK_E_TIMEOUT)
        {
            tcp->state = SDK_TCP_STATE_TIMEOUT;
        }
        break;
    }
    case SDK_TCP_STATE_TIMEOUT:
    {
        tcp->state = SDK_TCP_STATE_IDLE;
        break;
    }
    default:
        tcp->state = SDK_TCP_STATE_IDLE;
        break;
    }
}

sdk_err_t sdk_tcp_init(struct sdk_tcp *tcp, void *device, uint8_t *recv_fifo_buf, uint32_t recv_fifo_buf_size)
{
    tcp->socket = -1;

    // device
    tcp->device = device;

    // tcp fsm
    tcp->state = SDK_TCP_STATE_IDLE;
    sdk_os_event_init(&tcp->event);

    tcp->open_state = SDK_TCP_OPEN_STATE_IDLE;
    sdk_ringbuffer_init(&tcp->recv_fifo, recv_fifo_buf, recv_fifo_buf_size);
    sdk_os_event_send(&tcp->event, SDK_TCP_EVENT_OPEN);
    return SDK_OK;
}

sdk_err_t sdk_tcp_deinit(struct sdk_tcp *tcp)
{
    tcp->state = SDK_TCP_STATE_IDLE;
    tcp->open_state = SDK_TCP_OPEN_STATE_IDLE;
    sdk_ringbuffer_reset(&tcp->recv_fifo);
    sdk_os_event_send(&tcp->event, SDK_TCP_EVENT_CLOSE);

    return SDK_OK;
}


sdk_err_t sdk_tcp_recv(struct sdk_tcp *tcp, uint8_t *buf, uint32_t len)
{
    
}

sdk_err_t sdk_tcp_send(struct sdk_tcp *tcp, const uint8_t *buf, uint32_t len);
{
    tcp->send_buf = buf;
    tcp->send_len = len;
    sdk_os_event_send(&tcp->event, SDK_TCP_EVENT_SEND);
}
