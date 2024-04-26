/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-04-18     rgw             first version
 */

#include "apbus.h"
#include "pt.h"

#define DBG_TAG "bus"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

int apbus_init(apbus_t *ctx)
{
    return 0;
}

#include "sdk_board.h"
uint32_t apbus_get_ticks(void)
{
    return sdk_hw_get_systick();
}

apbus_t *apbus_find_dest_dev(uint8_t dest_id, apbus_t *apbus_dev_table[], int dev_num)
{
    for (int i = 0; i < dev_num; i++)
    {
        for(int j = 0; j < APBUS_MAX_SUB_ID; j++)
        {
            if(apbus_dev_table[i]->sub_id[j] == dest_id) 
            {
                return apbus_dev_table[i];
            }
        }
    }
    return NULL;
}

/**
 * @brief 获取指令的回复body
 * 
 * @param ctx 
 * @param cmd_fun 
 * @param cmd_index 
 * @param body 存储回复body的buf
 * @param p_body_len 回复body长度
 * @return int 
 */
static int apbus_body_get(apbus_t *ctx, uint8_t cmd_fun, uint16_t cmd_index)
{
    if (ctx->msg_body.body_recv == 1 && ctx->msg_body.cmd_fun == cmd_fun)
    {
        // CMD_CMAP_ACK_RD指令需要判断index是否匹配
        if (cmd_fun == CMD_CMAP_ACK_RD && ctx->msg_body.cmd_index != cmd_index)
        {
            LOG_E("got %02x exp %02x\r\n", ctx->msg_body.cmd_index, cmd_index);
            return 0;
        }
        ctx->msg_body.body_recv = 0; // must clear first !!!
        return 1;
    }
    return 0;
}

/*******************
 * 
 * 请求
 * 
 * ****************/
static int apbus_read_reg_send(apbus_t *ctx, uint8_t src_id, uint8_t dest_id, uint16_t index, int read_reg_num)
{
    uint8_t msg[APBUS_MAX_MSG_LEN] = {0};
    int msg_len;
    uni_data_16_t data16 = {0};
    int ret = 0;

    msg_meta_t msg_meta_read = {0};
    data16.data_16 = read_reg_num * 2;

    msg_meta_read.src_id = src_id;
    msg_meta_read.dest_id = dest_id;
    msg_meta_read.cmd_func = CMD_CMAP_RD;
    msg_meta_read.uni_index.data_16 = index;
    msg_meta_read.uni_body_len.data_16 = 2;
    msg_meta_read.body[0] = data16.data_8[0];
    msg_meta_read.body[1] = data16.data_8[1];

    ctx->backend->make_send_msg(ctx, &msg_meta_read, msg, &msg_len);
    ctx->backend->write(ctx, msg, msg_len);
    
    return ret;
}

/**
 * @brief 读取reg，pt线程
 * 
 * @param ctx 
 * @param pt pt句柄
 * @param src_id 
 * @param dest_id 
 * @param index 
 * @param read_reg_num 读取寄存器个数，注意不是数据段长度，一个寄存器长度为2个字节
 * @param reg 
 * @param reg_num 返回读取寄存器个数
 */
PT_THREAD(apbus_read_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *read_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int read_reg_num, uint16_t *reg, int *reg_num))
{
    uni_data_16_t data16 = {0};
    int bodyget_ret = 0;
    int swtimer_ret = 0;

    PT_BEGIN(pt);

    apbus_read_reg_send(ctx, src_id, dest_id, index, read_reg_num);
    swtimer_set(read_reg_timer, response_timeout);
    PT_YIELD_UNTIL(pt, ((bodyget_ret = apbus_body_get(ctx, CMD_CMAP_ACK_RD, index)) != 0) || ((swtimer_ret = swtimer_expired(read_reg_timer)) != 0));
    if(swtimer_ret == 1)
    {
        // LOG_D("read reg timeout: src[0x%02x] dest[0x%02x] index[0x%04x]\n", src_id, dest_id, index);
        *reg_num = APBUS_ERR_MSG_TIMEOUT;
    }
    else if(bodyget_ret == 1)
    {
        for (int i = 0; i < ctx->msg_body.body_len / 2; i++)
        {
            data16.data_8[0] = ctx->msg_body.body[i * 2];
            data16.data_8[1] = ctx->msg_body.body[i * 2 + 1];
            reg[i] = data16.data_16;
        }
        *reg_num = ctx->msg_body.body_len / 2;
    }
    PT_YIELD_FLAG = PT_YIELD_FLAG;
    PT_END(pt);
}


static int apbus_write_reg_send(apbus_t *ctx, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int ack)
{
    uint8_t msg[APBUS_MAX_MSG_LEN] = {0};
    int msg_len;
    uni_data_16_t data16 = {0};
    int ret = 0;
    msg_meta_t msg_meta_write = {0};

    msg_meta_write.src_id = src_id;
    msg_meta_write.dest_id = dest_id;
    if (ack)
        msg_meta_write.cmd_func = CMD_CMAP_WR;
    else
        msg_meta_write.cmd_func = CMD_CMAP_WR_NR;
    msg_meta_write.uni_index.data_16 = index;
    msg_meta_write.uni_body_len.data_16 = write_reg_num * 2;

    for (int i = 0; i < write_reg_num; i++)
    {
        data16.data_16 = reg[i];
        msg_meta_write.body[i * 2] = data16.data_8[0];
        msg_meta_write.body[i * 2 + 1] = data16.data_8[1];
    }

    ctx->backend->make_send_msg(ctx, &msg_meta_write, msg, &msg_len);
    ctx->backend->write(ctx, msg, msg_len);

    return ret;
}

PT_THREAD(apbus_write_NR_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *write_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int *ret_write))
{
    PT_BEGIN(pt);

    apbus_write_reg_send(ctx, src_id, dest_id, index, write_reg_num, reg, 0);
    swtimer_set(write_reg_timer, response_timeout);
    *ret_write = 0;
    PT_YIELD_FLAG = PT_YIELD_FLAG;
    PT_END(pt);
}

PT_THREAD(apbus_write_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *write_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int *ret_write))
{
    int bodyget_ret = 0;
    int swtimer_ret = 0;

    PT_BEGIN(pt);

    apbus_write_reg_send(ctx, src_id, dest_id, index, write_reg_num, reg, 1);
    swtimer_set(write_reg_timer, response_timeout);
    PT_YIELD_UNTIL(pt, ((bodyget_ret = apbus_body_get(ctx, CMD_CMAP_ACK_WR, index)) != 0) || ((swtimer_ret = swtimer_expired(write_reg_timer)) != 0));
    if(swtimer_ret == 1)
    {
        LOG_D("write reg timeout: src[0x%02x] dest[0x%02x] index[0x%04x]\n", src_id, dest_id, index);
        *ret_write = APBUS_ERR_MSG_TIMEOUT;
    }
    else if(bodyget_ret == 1)
    {
        if(ctx->msg_body.body_len == 1)
        {
            *ret_write = ctx->msg_body.body[0];
        }
        else
        {
            *ret_write = APBUS_ERR_WRERROR;
        }
    }
    PT_YIELD_FLAG = PT_YIELD_FLAG;
    PT_END(pt);
}

/*******************
 * 
 * 回复
 * 
 * ****************/
int apbus_send_read_ack(apbus_t *ctx, msg_meta_t *p_msg_meta, int read_len, uint16_t *reg_tab)
{
    uint8_t msg[APBUS_MAX_MSG_LEN] = {0};
    int msg_len;
    msg_meta_t msg_meta_ack = {0};

    msg_meta_ack.src_id = p_msg_meta->dest_id; //回复交换ID
    msg_meta_ack.dest_id = p_msg_meta->src_id;
    msg_meta_ack.cmd_func = CMD_CMAP_ACK_RD;
    msg_meta_ack.uni_index = p_msg_meta->uni_index;
    msg_meta_ack.uni_body_len.data_16 = read_len;
    memcpy(msg_meta_ack.body, &reg_tab[msg_meta_ack.uni_index.data_16], msg_meta_ack.uni_body_len.data_16);

    ctx->backend->make_send_msg(ctx, &msg_meta_ack, msg, &msg_len);
    ctx->backend->write(ctx, msg, msg_len);
    // LOG_D("[%02x->%02x] body_len %d, cmd %02x, index %02x\r\n",
    //       msg_meta_ack.src_id, msg_meta_ack.dest_id, msg_meta_ack.uni_body_len.data_16, msg_meta_ack.cmd_func, msg_meta_ack.uni_index.data_16);

    return APBUS_ERR_NO;
}

int apbus_send_write_ack(apbus_t *ctx, msg_meta_t *p_msg_meta, uint8_t write_ret)
{
    uint8_t msg[APBUS_MAX_MSG_LEN] = {0};
    int msg_len;
    msg_meta_t msg_meta_ack = {0};

    msg_meta_ack.src_id = p_msg_meta->dest_id; //回复交换ID
    msg_meta_ack.dest_id = p_msg_meta->src_id;
    msg_meta_ack.cmd_func = CMD_CMAP_ACK_WR;
    msg_meta_ack.uni_index = p_msg_meta->uni_index;
    msg_meta_ack.body[0] = write_ret;
    msg_meta_ack.uni_body_len.data_16 = 1;

    ctx->backend->make_send_msg(ctx, &msg_meta_ack, msg, &msg_len);
    ctx->backend->write(ctx, msg, msg_len);

    return APBUS_ERR_NO;
}


/**
 * parser
 * 
*/
int apbus_received_ack(apbus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta)
{
    memcpy(ctx->msg_body.body, p_msg_meta->body, p_msg_meta->uni_body_len.data_16);
    ctx->msg_body.body_len = p_msg_meta->uni_body_len.data_16;
    ctx->msg_body.body_recv = 1;
    ctx->msg_body.cmd_fun = p_msg_meta->cmd_func;
    ctx->msg_body.cmd_index = p_msg_meta->uni_index.data_16;

    // LOG_D("apbus_received_ack [0x%02x],len = %d\n", p_msg_meta->cmd_func, p_msg_meta->uni_body_len.data_16);
    // LOG_D("msg.body\n");
    // LOG_HEX(64, ctx->msg_body.body, ctx->msg_body.body_len);

    return 0;
}

/**
 * @brief 读取完整的一帧数据
 * 
 * @param ctx           apbus实例指针
 * @param p_msg_meta    msg_meta内容
 * @param pp_body       存储body开始地址的指针（复用下面的msg buf）
 * @param p_body_len    body长度
 * @param msg           存储数据帧的buf
 * @param wait          是否等待模式
 * @return int          读取的长度或错误码
 */

static PT_THREAD(apbus_receive_msg(apbus_t *ctx, msg_meta_t *p_msg_meta, uint8_t *msg, int *ret_rl, int wait))
{
    int length_to_read;
    uint32_t rl = 0;
    int ret = 0;

    length_to_read = ctx->backend->header_length + ctx->backend->checksum_length + ctx->backend->max_data_length;

    PT_BEGIN(&ctx->pt_receive_msg);

    PT_SPAWN(&ctx->pt_receive_msg, &ctx->pt_backend_read, ctx->backend->read(ctx, msg, length_to_read, &rl, wait));
    if (rl > 0)
    {
    //    LOG_D("recv(hex): ");
    //    LOG_HEX(64, msg, rl);
        ret = ctx->backend->check_integrity(ctx, msg, rl, p_msg_meta);
        if (ret == APBUS_ERR_NO)
            *ret_rl = rl;
        else
            *ret_rl = ret;
    }
    
    PT_YIELD_FLAG = PT_YIELD_FLAG;
    PT_END(&ctx->pt_receive_msg);
}


PT_THREAD(apbus_recv_poll(apbus_t *ctx, int host_id))
{
    msg_meta_t msg_meta;
    int rl = 0;
    int ret = 0;

    PT_BEGIN(&ctx->pt_recv_poll);

    PT_SPAWN(&ctx->pt_recv_poll, &ctx->pt_receive_msg, apbus_receive_msg(ctx, &msg_meta, ctx->msg, &rl, 0));
    if (rl > 0)
    {
        // LOG_D("[%02x->%02x] body_len %d, cmd %02x, index %02x\r\n",
        //             msg_meta.src_id, msg_meta.dest_id, msg_meta.uni_body_len.data_16, msg_meta.cmd_func, msg_meta.uni_index.data_16);

        if (ctx->parser == NULL)
        {
            ret = APBUS_ERR_NULL_PARSER;
            LOG_E("apbus_recv_poll return APBUS_ERR_NULL_PARSER\n");
        }

        if (msg_meta.dest_id == host_id) // 目标是本机地址
        {
            for (int i = 0; i < APBUS_CMD_MAX_OP; i++)
            {
                if (ctx->parser->own_cmd_ops[i].cmd_func == 0 
                    && ctx->parser->own_cmd_ops[i].cmd_handler == NULL)
                {
                    //LOG_E("own_cmd_ops[%d], cmd_handler = NULL", i);
                }

                if (ctx->parser->own_cmd_ops[i].cmd_func == msg_meta.cmd_func
                    && ctx->parser->own_cmd_ops[i].cmd_handler != NULL)
                {
                    ret = ctx->parser->own_cmd_ops[i].cmd_handler(ctx, ctx->msg, &msg_meta);
                    if(ret != 0)
                    {
                        LOG_E("apbus_recv_poll return %d\n", ret);
                    }
                }
            }
        }
        else // 目标不是本机地址
        {
            if (ctx->parser->others_cmd_handler != NULL)
            {
                ret = ctx->parser->others_cmd_handler(ctx, ctx->msg, &msg_meta);
                if(ret != 0)
                {
                    LOG_E("apbus_recv_poll return %d\n", ret);
                }
            }
        }
    }

    PT_YIELD_FLAG = PT_YIELD_FLAG;
    PT_END(&ctx->pt_recv_poll);
}
