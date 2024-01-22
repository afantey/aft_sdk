/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-23     rgw             first version
 */

#ifndef __SDK_TCP_H
#define __SDK_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aft_sdk.h"

// tcp event
#define SDK_TCP_EVENT_OPEN      BIT0
#define SDK_TCP_EVENT_CONNECT   BIT1
#define SDK_TCP_EVENT_CLOSE     BIT2
#define SDK_TCP_EVENT_SEND      BIT3
#define SDK_TCP_EVENT_SEND_OK   BIT4
#define SDK_TCP_EVENT_SEND_FAIL BIT5
#define SDK_TCP_EVENT_RECV      BIT6

enum sdk_tcp_state
{
    SDK_TCP_STATE_IDLE,
    SDK_TCP_STATE_OPEN,
    SDK_TCP_STATE_CONNECT,
    SDK_TCP_STATE_CLOSE,
    SDK_TCP_STATE_SEND,
    SDK_TCP_STATE_RECV,
    
    SDK_TCP_STATE_OK,
    SDK_TCP_STATE_TIMEOUT,
    SDK_TCP_STATE_ERROR,
};


struct sdk_tcp;

struct sdk_tcp_ops
{
    sdk_err_t (*init)(struct sdk_tcp *tcp);
    sdk_err_t (*open)(struct sdk_tcp *tcp);
    sdk_err_t (*connect)(struct sdk_tcp *tcp, const char *ip, uint16_t port);
    sdk_err_t (*close)(struct sdk_tcp *tcp);
    sdk_err_t (*send)(struct sdk_tcp *tcp, const char *buf, uint32_t len);
    sdk_err_t (*recv)(struct sdk_tcp *tcp, char *buf, uint32_t len);
};

enum sdk_tcp_open_state
{
    SDK_TCP_OPEN_STATE_IDLE,
    SDK_TCP_OPEN_STATE_START,
    SDK_TCP_OPEN_STATE_OK,
    SDK_TCP_OPEN_STATE_ERROR
};

struct sdk_tcp
{
    int32_t socket;

    //device
    void *device;

    // server config before open
    char *ip;
    uint16_t port;
 
    // init in send
    uint8_t *send_buf;
    uint32_t send_len;

    //init
    struct sdk_ringbuffer recv_fifo;

    // tcp fsm
    enum sdk_tcp_state state;
    struct sdk_os_event event;

    enum sdk_tcp_open_state open_state;
    sdk_os_mutex_t lock;
    
    struct sdk_tcp_ops ops;
};

sdk_err_t sdk_tcp_init(struct sdk_tcp *tcp, uint8_t *recv_fifo_buf, uint32_t recv_fifo_buf_size);
sdk_err_t sdk_tcp_deinit(struct sdk_tcp *tcp);

sdk_err_t sdk_tcp_open(struct sdk_tcp *tcp);
sdk_err_t sdk_tcp_connect(struct sdk_tcp *tcp, const char *ip, uint16_t port);
sdk_err_t sdk_tcp_close(struct sdk_tcp *tcp);

sdk_err_t sdk_tcp_send(struct sdk_tcp *tcp, const uint8_t *buf, uint32_t len);
sdk_err_t sdk_tcp_recv(struct sdk_tcp *tcp, uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_TCP_H */