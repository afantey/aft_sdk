/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-23     rgw             first version
 */

#include "aft_sdk.h"
#include "sdk_board.h"
#include "sdk_tcp.h"
#include "drv_nb_bc260.h"

enum bc260_at_cmd_tcp_step
{
    CMD_TCP_STEP_START,
    CMD_TCP_STEP_QICFG,

    CMD_TCP_STEP_OK,
    CMD_TCP_STEP_ERROR,
    CMD_TCP_STEP_TIMEOUT,
    CMD_TCP_STEP_BUFF_FULL,
};


static enum bc260_at_cmd_tcp_step open_step = CMD_TCP_STEP_START;
static enum bc260_at_cmd_tcp_step connect_step = CMD_TCP_STEP_START;
static enum bc260_at_cmd_tcp_step send_step = CMD_TCP_STEP_START;
static enum bc260_at_cmd_tcp_step close_step = CMD_TCP_STEP_START;

static enum bc260_at_cmd_tcp_step bc260_cmd_tcp_open(struct at_client *client)
{
    at_resp_status_t status = AT_RESP_WAITING;
    switch (open_step)
    {
    case CMD_TCP_STEP_START:
        open_step = CMD_TCP_STEP_QICFG;
        break;
    case CMD_TCP_STEP_QICFG: // AT+QICFG="dataformat"[,<send_data_format>,<recv_data_format>]
        status = at_cmd_common_ex(client, 0, 1000, NULL, "AT+QICFG=\"dataformat\",0,0");
        if(status == AT_RESP_OK) 
        {
            open_step = CMD_TCP_STEP_OK;
        }
        else if(status < 0)
        {
            open_step = CMD_TCP_STEP_ERROR;
        }
        break;
       

    }

    
}

//init
sdk_err_t bc260_tcp_init(struct sdk_tcp *tcp)
{
    struct at_client *client = (struct at_client *)tcp->device->client;
    if(client == NULL)
    {
        return -SDK_ERROR;
    }
    at_obj_set_urc_table(client, urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
    return SDK_OK;
}

sdk_err_t bc260_tcp_open(struct sdk_tcp *tcp)
{
    sdk_err_t err = -SDK_E_BUSY;
    switch(tcp->open_state)
    {
    case SDK_TCP_OPEN_STATE_IDLE:
    }
}

sdk_err_t bc260_tcp_connect(struct sdk_tcp *tcp, const char *ip, uint16_t port)
{

}

sdk_err_t bc260_tcp_close(struct sdk_tcp *tcp)
{

}

sdk_err_t bc260_tcp_send(struct sdk_tcp *tcp, const char *buf, uint32_t len)
{

}

sdk_err_t bc260_tcp_recv(struct sdk_tcp *tcp, char *buf, uint32_t len)
{

}

struct sdk_tcp net_tcp_bc260 = {
    .socket = -1,
    .ops = {
            .open = bc260_tcp_open,
            .connect = bc260_tcp_connect,
            .close = bc260_tcp_close,
            .send = bc260_tcp_send,
            .recv = bc260_tcp_recv,
            }
};
