/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-06-26     rgw             first version
 */

#include "sdk_board.h"
#include "at_device.h"

#define DBG_TAG "at"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

// #define AT_SEND_CMD(client, retry, timeout_ms, parse_fun, ...)                                                     \
//     do                                                                                                             \
//     {                                                                                                              \
//         at_resp_status_t __status = at_cmd_common_ex((client), (retry), (timeout_ms), (parse_fun), (__VA_ARGS__)); \
//         if (__status == AT_RESP_OK)                                                                                \
//         {                                                                                                          \
//             status = __status;                                                                                     \
//         }                                                                                                          \
//         else if (__status == AT_RESP_WAITING)                                                                      \
//         {                                                                                                          \
//             return;                                                                                                \
//         }                                                                                                          \
//         else if (__status < 0)                                                                                     \
//         {                                                                                                          \
//             status = __status;                                                                                     \
//             goto __exit;                                                                                           \
//         }                                                                                                          \
//     } while (0)

void at_resp_set_info(struct at_resp *resp, size_t line_num, size_t line_len)
{
    resp->line_len = line_len;
    resp->line_num = line_num;
}

void at_obj_set_end_sign(struct at_client *client, char ch)
{
    if (client == NULL)
    {
        LOG_E("input AT Client object is NULL, please create or get AT Client object!");
        return;
    }

    client->resp->end_sign = ch;
}

void at_obj_set_urc_table(struct at_client *client, struct at_urc *urc_table, size_t table_sz)
{
    struct at_urc *current = &client->urc_list;
    struct at_urc *check = NULL;

    // 遍历urc_table数组
    for (int i = 0; i < table_sz; i++)
    {
        // 检查链表中是否已经存在相同的节点
        check = &client->urc_list;
        while (check != NULL)
        {
            if (check == &urc_table[i])
            {
                // 如果找到相同的节点，跳过这个节点
                break;
            }
            check = check->next;
        }

        // 如果没有找到相同的节点，将urc_table[i]添加到链表的末尾
        if (check == NULL)
        {
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = &urc_table[i];
        }
    }
}


static const struct at_urc *get_urc_obj(struct at_client *client)
{
    size_t prefix_len, suffix_len;
    size_t bufsz;
    char *buffer = NULL;    
    const struct at_urc *urc = NULL;

    if (client->urc_list.next == NULL)
    {
        return NULL;
    }

    buffer = client->recv_line_buf;
    bufsz = client->recv_line_len;

    urc = client->urc_list.next;

    while(urc != NULL)
    {
        prefix_len = strlen(urc->cmd_prefix);
        suffix_len = strlen(urc->cmd_suffix);
        if (bufsz < prefix_len + suffix_len)
        {
            urc = urc->next;
            continue;
        }
        if ((prefix_len ? !strncmp(buffer, urc->cmd_prefix, prefix_len) : 1) &&
            (suffix_len ? !strncmp(buffer + bufsz - suffix_len, urc->cmd_suffix, suffix_len) : 1))
        {
            return urc;
        }
        urc = urc->next;
    }

    return NULL;
}

const char *at_resp_get_line_by_kw(struct at_resp *resp, const char *keyword)
{
    char *resp_buf = resp->buf;
    char *resp_line_buf = NULL;
    size_t line_num = 1;

    for (line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (strstr(resp_buf, keyword))
        {
            resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

int at_resp_parse_line_args_by_kw(struct at_resp *resp, const char *keyword, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = NULL;

    if ((resp_line_buf = at_resp_get_line_by_kw(resp, keyword)) == NULL)
    {
        return -1;
    }
    va_start(args, resp_expr);
    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);
    va_end(args);

    return resp_args_num;
}


static enum at_resp_rl_state at_recv_readline(struct at_client *client, struct at_resp *resp)
{
    int rl = 0;
    char ch = 0;
    bool is_full = false;

    switch (resp->rl_state)
    {
    case AT_RESP_RL_STAT_START:
        resp->rl_state = AT_RESP_RL_STAT_INIT;
        break;
    case AT_RESP_RL_STAT_INIT:
        memset(client->recv_line_buf, 0x00, client->line_bufsz);
        client->recv_line_len = 0;
        resp->rl_state = AT_RESP_RL_STAT_RECV;
        resp->read_len = 0;
        break;
    case AT_RESP_RL_STAT_RECV:
        rl = sdk_uart_read(client->uart_port, (uint8_t *)&ch, 1);
        if(rl == 1)
        {
            if (resp->read_len < client->line_bufsz)
            {
                client->recv_line_buf[resp->read_len++] = ch;
                client->recv_line_len = resp->read_len;
            }
            else
            {
                is_full = true;
            }
        
            /* is newline or URC data */
            if ((ch == '\n' && resp->last_ch == '\r' && resp->read_len >= resp->line_len) ||
                (resp->end_sign != 0 && ch == resp->end_sign) ||
                get_urc_obj(client))
            {
                if (is_full)
                {
                    LOG_E("read line failed. The line data length is out of buffer size(%d)!", client->line_bufsz);
                    memset(client->recv_line_buf, 0x00, client->line_bufsz);
                    client->recv_line_len = 0;
                    resp->rl_state = AT_RESP_RL_STAT_BUFF_FULL;
                    break;
                }
                resp->rl_state = AT_RESP_RL_STAT_READ_LINE_OK;
                break;
            }
            resp->last_ch = ch;
        }
        break;
    case AT_RESP_RL_STAT_READ_LINE_OK:
        break;
    case AT_RESP_RL_STAT_BUFF_FULL:
        break;
    default:
        LOG_E("AT response read line state error! state %d", resp->rl_state);
        break;
    }

    return resp->rl_state;
}

static void at_parser_start(struct at_client *client)
{
    client->resp->parser_state = AT_RESP_PARSER_STAT_START;
}

static enum at_resp_parser_state at_parser(struct at_client *client, struct at_resp *resp)
{
    const struct at_urc *urc;
    
    SDK_ASSERT(client != NULL);
    SDK_ASSERT(resp != NULL);

    // enum at_resp_rl_state resp_state;
    switch (resp->parser_state)
    {
    case AT_RESP_PARSER_STAT_START:
        resp->rl_state = AT_RESP_RL_STAT_START;
        resp->parser_state = AT_RESP_PARSER_STAT_READ_LINE;
        resp->buf_len = 0;
        resp->line_counts = 0;
        break;
    case AT_RESP_PARSER_STAT_READ_LINE:
    {
        enum at_resp_rl_state rl_state = at_recv_readline(client, resp);
        if (rl_state == AT_RESP_RL_STAT_READ_LINE_OK)
        {
            resp->parser_state = AT_RESP_PARSER_STAT_PARSE_LINE;
        }
        else if (rl_state == AT_RESP_RL_STAT_BUFF_FULL)
        {
            resp->resp_status = AT_RESP_BUFF_FULL;
            resp->parser_state = AT_RESP_PARSER_STAT_PARSE_OK;
        }
        break;
    }
    case AT_RESP_PARSER_STAT_PARSE_LINE:
        if ((urc = get_urc_obj(client)) != NULL)
        {
            /* current receive is request, try to execute related operations */
            if (urc->func != NULL)
            {
                urc->func(client, client->recv_line_buf, client->recv_line_len);
            }
            resp->resp_status = AT_RESP_OK;
            resp->parser_state = AT_RESP_PARSER_STAT_START;
            break;
        }
        else
        {
            char end_ch = client->recv_line_buf[client->recv_line_len - 1];

            /* current receive is response */
            client->recv_line_buf[client->recv_line_len - 1] = '\0';
            if (resp->buf_len + client->recv_line_len < resp->buf_size)
            {
                /* copy response lines, separated by '\0' */
                memcpy(resp->buf + resp->buf_len, client->recv_line_buf, client->recv_line_len);

                /* update the current response information */
                resp->buf_len += client->recv_line_len;
                resp->line_counts++;
            }
            else
            {
                resp->resp_status = AT_RESP_BUFF_FULL;
                memset(resp->buf, 0x00, resp->buf_size);
                resp->buf_len = 0;
                resp->line_counts = 0;
                LOG_E("Read response buffer failed. The Response buffer size is out of buffer size(%d)!", resp->buf_size);
            }
            /* check response result */
            if ((resp->end_sign != 0) && (end_ch == resp->end_sign) && (resp->line_num == 0))
            {
                /* get the end sign, return response state END_OK.*/
                resp->resp_status = AT_RESP_OK;
            }
            else if (memcmp(client->recv_line_buf, AT_RESP_END_OK, strlen(AT_RESP_END_OK)) == 0 && resp->line_num == 0)
            {
                /* get the end data by response result, return response state END_OK. */
                resp->resp_status = AT_RESP_OK;
            }
            else if (strstr(client->recv_line_buf, AT_RESP_END_ERROR) || (memcmp(client->recv_line_buf, AT_RESP_END_FAIL, strlen(AT_RESP_END_FAIL)) == 0))
            {
                resp->resp_status = AT_RESP_ERROR;
            }
            else if (resp->line_counts == resp->line_num && resp->line_num)
            {
                /* get the end data by response line, return response state END_OK.*/
                resp->resp_status = AT_RESP_OK;
            }
            else
            {
                resp->rl_state = AT_RESP_RL_STAT_START;
                resp->parser_state = AT_RESP_PARSER_STAT_READ_LINE;
                break;
            }
        }

        sdk_os_sem_release(&client->resp_notice);
        resp->parser_state = AT_RESP_PARSER_STAT_PARSE_OK;
        break;
    case AT_RESP_PARSER_STAT_PARSE_OK:
        resp->parser_state = AT_RESP_PARSER_STAT_START;
        break;
    default:
        LOG_E("AT response parser state error! state %d", resp->parser_state);
        break;
    }

    return resp->parser_state;
}

void at_parser_poll(struct at_client *client)
{
    struct at_resp *resp = client->resp;

    enum at_resp_parser_state parser_state = at_parser(client, resp);
    if (parser_state == AT_RESP_PARSER_STAT_PARSE_OK)
    {
        at_parser_start(client);
    }
}

int at_client_init(struct at_client *client, sdk_uart_t *uart_port, uint8_t *recv_line_buf, size_t line_bufsz)
{
    int result = SDK_OK;
//    sdk_err_t open_result = SDK_OK;

    client->uart_port = uart_port;
    client->recv_line_buf = (char *)recv_line_buf;
    client->line_bufsz = line_bufsz;

    sdk_os_sem_init(&client->resp_notice, 0);

    if (client->uart_port)
    {
//        open_result = sdk_uart_open(client->uart_port, );

//        rt_device_set_rx_indicate(client->device, at_client_rx_ind);
    }
    else
    {
        LOG_E("AT client initialize failed!");
        result = -SDK_ERROR;
        goto __exit;
    }

__exit:
    if (result == SDK_OK)
    {
        at_parser_start(client);
    }

    return result;
}

at_resp_status_t at_resp_get_ok(struct at_client *client, int timeout_ms)
{
    at_resp_status_t status = AT_RESP_WAITING;
    sdk_err_t result = sdk_os_sem_take(&client->resp_notice, timeout_ms);
    if (result == SDK_OK)
    {
        status = client->resp->resp_status;
    }
    else if (result == -SDK_E_TIMEOUT)
    {
        status = AT_RESP_TIMEOUT;
    }

    return status;
}

static enum at_cmd_state AT_CMD_SEND(struct at_client *client, int timeout_ms, char *cmd_str, int cmd_len)
{
    struct at_cmd *cmd = client->cmd;

    switch (cmd->state_send)
    {
    case AT_CMD_STAT_START:
        cmd->state_send = AT_CMD_STAT_SEND;
        break;
    case AT_CMD_STAT_SEND:
    {
        // LOG_RAW("%s", cmd_str);
        for(int i = 0; i < cmd_len; i++)
        {
            LOG_RAW("%c", cmd_str[i]);
        }
        sdk_os_sem_init(&client->resp_notice, 0);
        sdk_uart_write(client->uart_port, (uint8_t *)cmd_str, cmd_len);
        cmd->state_send = AT_CMD_STAT_WAIT_REV;
        break;
    }
    case AT_CMD_STAT_WAIT_REV:
    {
        sdk_err_t status = sdk_os_sem_take(&client->resp_notice, timeout_ms);
        if (status == SDK_OK)
        {
            cmd->state_send = AT_CMD_STAT_REVOK;
        }
        else if (status == -SDK_E_TIMEOUT)
        {
            cmd->state_send = AT_CMD_STAT_TIMEOUT;
        }
        else
        {
            cmd->state_send = AT_CMD_STAT_WAIT_REV;
        }
        break;
    }
    case AT_CMD_STAT_REVOK:
    case AT_CMD_STAT_TIMEOUT:
        cmd->state_send = AT_CMD_STAT_START;
        break;
    default:
        LOG_E("AT cmd state error! state %d", cmd->state_send);
        break;
    }
    return cmd->state_send;
}

at_resp_status_t at_cmd_common(struct at_client *client, char *format, ...)
{
    at_resp_status_t status = AT_RESP_WAITING;

    char cmd_str[AT_CMD_MAX_LEN + 1] = {0};
    va_list va;
    uint32_t cmd_len;

    va_start(va, format);
    cmd_len = vsnprintf(cmd_str, sizeof(cmd_str) - 1, format, va);
    va_end(va);

    if (cmd_len > AT_CMD_MAX_LEN)
    {
        LOG_E("AT cmd len(%d) is out of range(%d)!", cmd_len, AT_CMD_MAX_LEN);
    }

    enum at_cmd_state state_send = AT_CMD_SEND(client, 5000, cmd_str, cmd_len);
    if (state_send == AT_CMD_STAT_REVOK)
    {
        status = client->resp->resp_status;
    }

    return status;
}

at_resp_status_t at_cmd_common_ex(struct at_client *client, int retry, int timeout_ms, at_resp_status_t (*parse_func)(struct at_resp *resp), char *format, ...)
{
    struct at_cmd *cmd = client->cmd;
    struct at_resp *resp = client->resp;
    at_resp_status_t status = AT_RESP_WAITING;
    int interval_ms = 1000;
    switch (cmd->state_common)
    {
    case AT_CMD_STAT_START:
        cmd->trycnt_send = 0;
        cmd->state_common = AT_CMD_STAT_SEND;
        break;
    case AT_CMD_STAT_SEND:
    {
        char cmd_str[AT_CMD_MAX_LEN + 1] = {0};
        va_list va;
        uint32_t cmd_len;

        va_start(va, format);
        cmd_len = vsnprintf(cmd_str, sizeof(cmd_str) - 1, format, va);
        va_end(va);

        if (cmd_len > AT_CMD_MAX_LEN)
        {
            LOG_E("AT cmd len(%d) is out of range(%d)!", cmd_len, AT_CMD_MAX_LEN);
        }

        enum at_cmd_state state_send = AT_CMD_SEND(client, timeout_ms, cmd_str, cmd_len);
        if (state_send == AT_CMD_STAT_REVOK)
        {
            status = resp->resp_status;
            if (status == AT_RESP_OK)
            {
                if(parse_func != NULL)
                {
                    status = parse_func(resp);
                    if (status == AT_RESP_OK)
                    {
                        cmd->state_common = AT_CMD_STAT_START;
                    }
                    else
                    {
                        status = AT_RESP_WAITING;
                        cmd->state_common = AT_CMD_STAT_RETRY;
                    }
                }
                else
                {
                    cmd->state_common = AT_CMD_STAT_START;
                }
            }
            else if(status == AT_RESP_ERROR)
            {
                status = AT_RESP_WAITING;
                cmd->state_common = AT_CMD_STAT_RETRY;
            }
        }
        else if(state_send == AT_CMD_STAT_TIMEOUT)
        {
            status = AT_RESP_WAITING;
            cmd->state_common = AT_CMD_STAT_RETRY;
        }
        break;
    }
    case AT_CMD_STAT_RETRY:
        if (cmd->trycnt_send >= retry)
        {
            cmd->state_common = AT_CMD_STAT_TIMEOUT;
        }
        else
        {
            cmd->trycnt_send++;
            status = AT_RESP_WAITING;
            cmd->state_common = AT_CMD_STAT_WAIT_SEND;
            swtimer_set(&cmd->timer_send_wait, interval_ms);
        }
        break;
    case AT_CMD_STAT_WAIT_SEND:
        if (swtimer_expired(&cmd->timer_send_wait))
        {
            status = AT_RESP_WAITING;
            cmd->state_common = AT_CMD_STAT_SEND;
            break;
        }
        break;
    case AT_CMD_STAT_TIMEOUT:
        status = AT_RESP_TIMEOUT;
        cmd->state_common = AT_CMD_STAT_START;
        break;
    default:
        LOG_E("AT cmd state error! state %d", cmd->state_common);
        break;
    }

    return status;
}
