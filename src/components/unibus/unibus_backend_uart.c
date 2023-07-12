/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#include "unibus.h"
#include "unibus_board.h"
#include "sdk_board.h"

#define DBG_TAG "bus.uart"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

//////////////////////////需要不同平台对接的函数///////////////////////////////////////////////////
// int unibus_serial_read(void *pctx, uint8_t *rbuff, int len, int wait);
// int unibus_serial_write(void *pctx, uint8_t *pbuff, int len);
///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 串口发
 * 
 * @param pctx 
 * @param pbuff 
 * @param len 
 * @return int 
 */
static int unibus_serial_write(void *pctx, const uint8_t *pbuff, int len)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    return sdk_uart_write(ctx->port, pbuff, len);
}

/**
 * @brief 从缓存中读取一帧协议
 * 
 * @param pctx 
 * @param rbuff 
 * @param len 
 * @return int 
 */

struct unibus_serial_read_pt_data
{
    uint32_t sum;
    int timeout_ms;
    int remaining_len;
    int step; //0 验证阶段  1 读取阶段
    uni_data_16_t uni_body_len;
};

static struct unibus_serial_read_pt_data serial_pt_data = {
    .sum = 0,
    .timeout_ms = 0,
    .remaining_len = 0,
    .step = 0,
    .uni_body_len = {0},
};
#define LOOP_DELAY 1
static PT_THREAD(unibus_serial_read(void *pctx, uint8_t *rbuff, int len, uint32_t *psum, int wait))
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;
    uint32_t rl = 0;
    struct unibus_serial_read_pt_data *pt_data = (struct unibus_serial_read_pt_data *)ctx->backend->pt_data; 

    PT_BEGIN(&ctx->pt_backend_read);

    memset(pt_data, 0, sizeof(struct unibus_serial_read_pt_data));
    pt_data->timeout_ms = ctx->response_timeout;
    
    do {
        swtimer_set(&ctx->swtimer_backend_read, LOOP_DELAY);
        if (len - pt_data->sum == 0)
            break;
        if (pt_data->step == 0) {
            rl = sdk_uart_read(ctx->port, rbuff + pt_data->sum, 1);
            if (rl == 1) {
                switch (pt_data->sum) {
                case 0:
                    if (rbuff[0] == 0xAB)
                        pt_data->sum++;
                    break;
                case 1:
                    if (rbuff[1] == 0xBA)
                        pt_data->sum++;
                    else if (rbuff[1] == 0xAB)
                        rbuff[0] = 0xAB;
                    else
                        pt_data->sum--;
                    break;
                case 2:
                    pt_data->uni_body_len.data_8[0] = rbuff[2];
                    pt_data->sum++;
                    break;
                  case 3:
                    pt_data->uni_body_len.data_8[1] = rbuff[3];
                    pt_data->remaining_len = pt_data->uni_body_len.data_16 + 7;
                    pt_data->sum++;
                    pt_data->step = 1;
                    break;
                default:
                    break;
                }
            } else if (wait == 0 && pt_data->sum == 0) //不等模式
            {
                break;
            }
        }
        if (pt_data->step == 1) {
            rl = sdk_uart_read(ctx->port, rbuff + pt_data->sum, pt_data->remaining_len);
            pt_data->sum += rl;
            pt_data->remaining_len -= rl;
            if (pt_data->sum == pt_data->uni_body_len.data_16 + 7 + 4) {
                break; //OK
            }
        }
        if (pt_data->timeout_ms == 0) {
            pt_data->sum = 0;
            break;
        }
        if (rl > 0) {
            continue;
        } else {
            PT_YIELD_UNTIL(&ctx->pt_backend_read, swtimer_expired(&ctx->swtimer_backend_read));
            pt_data->timeout_ms -= LOOP_DELAY;
            //LOG_D("timeout %d\n", pt_data->timeout_ms);
        }
    } while (pt_data->timeout_ms > 0);

    *psum = pt_data->sum;
    
    PT_END(&ctx->pt_backend_read);
}

/**
 * @brief 计算校验码
 * 
 * @param data 除AB，BA的内容
 * @param len 
 * @return uint16_t 
 */
static uint16_t checksumLRC(const uint8_t *data, int len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return -sum;
}

/**
 * @brief 制作串口发送帧
 * 
 * @param pctx 
 * @param p_msg_meta 
 * @param msg 
 * @param p_msg_len 
 * @return int 
 */
static int unibus_serial_make_msg(void *pctx, msg_meta_t *p_msg_meta, uint8_t *msg, int *p_msg_len)
{
    int offset = 0;
    uni_data_16_t checksum = {0};

    msg[offset++] = 0xAB;
    msg[offset++] = 0xBA;
    msg[offset++] = p_msg_meta->uni_body_len.data_8[0];
    msg[offset++] = p_msg_meta->uni_body_len.data_8[1];
    msg[offset++] = p_msg_meta->src_id;
    msg[offset++] = p_msg_meta->dest_id;
    msg[offset++] = p_msg_meta->cmd_func;
    msg[offset++] = p_msg_meta->uni_index.data_8[0];
    msg[offset++] = p_msg_meta->uni_index.data_8[1];
    for (int i = 0; i < p_msg_meta->uni_body_len.data_16; i++) {
        msg[offset++] = p_msg_meta->body[i];
    }
    checksum.data_16 = checksumLRC(msg + 2, offset - 2);
    msg[offset++] = checksum.data_8[0];
    msg[offset++] = checksum.data_8[1];
    *p_msg_len = offset;

    return UNI_ERR_NO;
}

/**
 * @brief 校验串口接收帧
 * 
 * @param pctx 
 * @param msg 
 * @param msg_len 
 * @param p_msg_meta 返回接收到的协议结构
 * @return int 
 */
static int unibus_serial_check_integrity(void *pctx, uint8_t *msg, int msg_len, msg_meta_t *p_msg_meta)
{
    unibus_t *ctx = (unibus_t *)pctx;
    uint16_t body_len = 0;
    uni_data_16_t checksum = {0};

    if (msg[0] != 0xAB || msg[1] != 0xBA)
    {
        return UNI_ERR_PROTO_BROKEN;
    }
    body_len = msg[2] + (msg[3] << 8);
    if (msg_len < (body_len + ctx->backend->header_length + ctx->backend->checksum_length) ||
        msg_len > (ctx->backend->max_data_length + ctx->backend->header_length + ctx->backend->checksum_length))
    {
        return UNI_ERR_PROTO_BROKEN;
    }
    checksum.data_16 = checksumLRC(msg + 2, body_len + 7); //7为协议中除帧头其余固定参数长度
    if (checksum.data_8[0] != msg[msg_len - 2] || checksum.data_8[1] != msg[msg_len - 1])
    {
        LOG_E("check orig[0]%02x [1]%02x, calc [0]%02x [1]%02x\n", msg[msg_len - 2], msg[msg_len - 1], checksum.data_8[0], checksum.data_8[1]);
        return UNI_ERR_PROTO_CHECK;
    }
    p_msg_meta->src_id = msg[4]; //原id
    p_msg_meta->dest_id = msg[5]; //目标id //接收时，目标id为接收端发送时候的原id
    p_msg_meta->cmd_func = msg[6]; //功能码
    p_msg_meta->uni_index.data_16 = msg[7] + (msg[8] << 8);
    for(int i = 0; i < body_len; i++)
    {
        p_msg_meta->body[i] = msg[ctx->backend->header_length + i];
    }
    p_msg_meta->uni_body_len.data_16 = body_len;
    return UNI_ERR_NO;
}

#if 0
static int _unibus_serial_flush(unibus_t *ctx)
{
    uint8_t buff[128];
    uint32_t l = 0;
    uint32_t sum = 0;
    while ((l = sdk_uart_read(ctx->port, buff, 128)) > 0)
    {
        sum += l;
    }
    return sum;
}
#endif

static int unibus_serial_connect(void *pctx)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    sdk_uart_open((sdk_uart_t *)ctx->port, 115200, 8, 'N', 1);
    sdk_uart_control((sdk_uart_t *)ctx->port, SDK_CONTROL_UART_ENABLE_INT, NULL);
    // sdk_uart_control((sdk_uart_t *)ctx->port, SDK_CONTROL_UART_ENABLE_DMA, NULL);
    //sdk_uart_write((sdk_uart_t *)ctx->port, (uint8_t *)"unibus!\n", strlen("unibus!\n"));
    return UNI_ERR_NO;
}

static void unibus_serial_close(void *pctx)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    sdk_uart_close((sdk_uart_t *)ctx->port);
}

const unibus_backend_t backend_uart = {
    .header_length    =  _UNIBUS_SERIAL_HEADER_LENGTH  ,
    .checksum_length  =  _UNIBUS_SERIAL_CHECKSUM_LENGTH,
    .max_data_length  =  _UNIBUS_SERIAL_MAX_DATA_LENGTH,
    .pt_data          = &serial_pt_data                ,
    .read             =  unibus_serial_read            ,
    .write            =  unibus_serial_write           ,
    .make_send_msg    =  unibus_serial_make_msg        ,
    .check_integrity  =  unibus_serial_check_integrity ,
    .receive          =  NULL                          ,
    .prepare_response =  NULL                          ,
    .connect          =  unibus_serial_connect         ,
    .close            =  unibus_serial_close           ,
};
