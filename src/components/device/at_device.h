/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-06-26     rgw             first version
 */

#ifndef __AT_DEVICE_H
#define __AT_DEVICE_H

#include "sdk_config.h"
#include "sdk_def.h"
#include "sdk_board.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef AT_CMD_MAX_LEN
#define AT_CMD_MAX_LEN  128
#endif

#define AT_RESP_END_OK                 "OK"
#define AT_RESP_END_ERROR              "ERROR"
#define AT_RESP_END_FAIL               "FAIL"
#define AT_END_CR_LF                   "\r\n"

enum at_cmd_state
{
    AT_CMD_STAT_START,
    AT_CMD_STAT_SEND,
    AT_CMD_STAT_WAIT_REV,
    AT_CMD_STAT_RETRY,
    AT_CMD_STAT_REVOK,
    AT_CMD_STAT_TIMEOUT
};

struct at_cmd
{
    enum at_cmd_state state;
    int trycnt;
    struct swtimer timer_wait_recv;

    enum at_cmd_state state_send;
    int trycnt_send;
    struct swtimer timer_send;
};

enum at_resp_rl_state
{
    AT_RESP_RL_STAT_START,
    AT_RESP_RL_STAT_INIT,
    AT_RESP_RL_STAT_RECV,
    AT_RESP_RL_STAT_READ_LINE_OK,
    AT_RESP_RL_STAT_BUFF_FULL,
};

enum at_resp_parser_state
{
    AT_RESP_PARSER_STAT_START,
    AT_RESP_PARSER_STAT_READ_LINE,
    AT_RESP_PARSER_STAT_PARSE_LINE,
    AT_RESP_PARSER_STAT_PARSE_OK,
};

enum at_resp_status
{
    AT_RESP_OK = 0,                   /* AT response end is OK */
    AT_RESP_WAITING = 1,              /* AT response end is OK */
    AT_RESP_ERROR = -1,               /* AT response end is ERROR */
    AT_RESP_TIMEOUT = -2,             /* AT response is timeout */
    AT_RESP_BUFF_FULL= -3,            /* AT response buffer is full */
};
typedef enum at_resp_status at_resp_status_t;

struct at_resp
{
    enum at_resp_rl_state rl_state;
    size_t read_len;    //current read length
    char last_ch;
    char end_sign;

    enum at_resp_parser_state parser_state;
    char *buf; //recv data until OK / ERROR / line limit or end_sign
    size_t buf_len;
    size_t buf_size;
    /* the number of setting response lines, it set by `at_create_resp()` function
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    size_t line_num;
    /* the count of received response lines */
    size_t line_counts;

    uint32_t urc_event;

    at_resp_status_t resp_status;
};

typedef struct at_response *at_response_t;

struct at_client;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY' request by AT server */
struct at_urc
{
    const char *cmd_prefix;
    const char *cmd_suffix;
    void (*func)(struct at_client *client, const char *data, size_t size);
};
typedef struct at_urc *at_urc_t;

struct at_urc_table
{
    size_t urc_size;
    const struct at_urc *urc;
};
typedef struct at_urc *at_urc_table_t;

struct at_client
{
    sdk_uart_t *uart_port;

    /* the current received one line data buffer */
    char *recv_line_buf;
    /* The length of the currently received one line data */
    size_t recv_line_len;
    /* The maximum supported receive data length */
    size_t recv_bufsz;
    sdk_os_mutex_t lock;

    struct at_urc_table urc_table;

    struct at_cmd *cmd;
    struct at_resp *resp;
};
typedef struct at_client *at_client_t;

void at_resp_set_info(struct at_resp *resp, size_t line_num);
int at_client_init(at_client_t client, sdk_uart_t *uart_port, uint8_t *recv_line_buf, size_t recv_bufsz);
void at_parser_start(at_client_t client);
enum at_resp_parser_state at_parser(at_client_t client, struct at_resp *resp);
enum at_cmd_state at_parser_wait_resp(struct at_client *client, int timeout_s);
void at_obj_set_end_sign(at_client_t client, char ch);
void at_obj_set_urc_table(at_client_t client, const struct at_urc *urc_table, size_t table_sz);
int at_resp_parse_line_args_by_kw(struct at_resp *resp, const char *keyword, const char *resp_expr, ...);
at_resp_status_t at_cmd_common(struct at_client *client, char *format, ...);
at_resp_status_t at_cmd_common_ex(struct at_client *client, int retry, int timeout_ms, at_resp_status_t (*parse_func)(struct at_resp *resp), char *format, ...);
enum at_cmd_state AT_DATA_SEND(struct at_client *client, int timeout_ms, uint8_t *data, int data_len);


#ifdef __cplusplus
}
#endif

#endif /* __AT_DEVICE_H */
