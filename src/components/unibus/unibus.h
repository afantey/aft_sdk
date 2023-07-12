/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#ifndef __UNIBUS_H
#define __UNIBUS_H

#include "sdk_board.h"
#include "sdk_config.h"
#include "sdk_def.h"
#include "sdk_soft_timer.h"
#include "pt.h"


#ifdef __cplusplus
extern "C" {
#endif


/******************************************************
 * 
 * UNIBUS配置参数
 * 
 ******************************************************/
#define UNIBUS_MAX_MSG_LEN  280
#define UNIBUS_MAX_BODY_LEN 256
#define UNIBUS_MAX_SUB_ID   4  //一个端口下最多可能会有多少个设备

#define UNIBUS_CMD_MAX_OP 16


/******************************************************
 * 
 * 通用命令字，在设备poll头文件中，可能有专用控制字
 * 
 ******************************************************/
// 控制表访问指令
#define CMD_CMAP_RD         0x01    // 读控制表数据
#define CMD_CMAP_WR         0x02    // 写控制表数据，需要应答。
#define CMD_CMAP_WR_NR      0x03    // 写控制表数据，无应答
#define CMD_CMAP_ACK_RD     0x04    // 对读取指令的响应包
#define CMD_CMAP_ACK_WR     0x05    // 对写指令的响应包
// 数据下载相关指令
#define CMD_IAP_BEGIN  0x07 //固件下载起始帧/车子有应答
#define CMD_IAP_TRANS  0x08 //固件下载数据帧/车子有应答
#define CMD_IAP_VERIFY 0x09 //固件下载校验帧/车子有应答
#define CMD_MCU_RESET  0x0A //芯片复位指令/车子无应答
#define CMD_IAP_ACK    0x0B //固件下载应答帧


/******************************************************
 * 
 * 协议操作失败原因
 * 
 ******************************************************/

#define CMD_CMAP_WRERROR_AUTH 0x01 //写入的地址没有写入权限
#define CMD_CMAP_WRERROR_BUSY 0x02 //正在操作控制表，不允许写入
#define CMD_CMAP_WRERROR_SIZE 0x03 //写入数据超范围
#define CMD_CMAP_WRERROR_FORM 0x04 //写入数据格式错误

#define IAP_BEGIN_OK        0x70
#define IAP_TRANS_OK        0x80
#define IAP_VERIFY_OK       0x90
#define IAPERROR_SIZE       0x71 // 	固件大小超范围
#define IAPERROR_ERASE      0x82 // 	擦除 Flash 失败
#define IAPERROR_WRITEFLASH 0x83 // 	写入 Flash 失败
#define IAPERROR_UNLOCK     0x74 // 	设备未处于可更新状态
#define IAPERROR_INDEX      0x85 // 	数据索引错误
#define IAPERROR_BUSY       0x86 // 	IAP 正忙（如正在进行 Flash 写入）
#define IAPERROR_FORM       0x87 // 	数据格式错误（如发送的数据长度不是 8 的整数倍）
#define IAPERROR_CRC        0x98 // 	数据校验失败
#define IAPERROR_RESET      0x09 // 	其他错误

/******************************************************
 * 
 * UNIBUS错误代码
 * 
 ******************************************************/
#define UNI_ERR_NO                   0
#define UNI_ERR_PROTO_BROKEN        -1
#define UNI_ERR_PROTO_CHECK         -2
#define UNI_ERR_CMD_INDEX           -3
#define UNI_ERR_CMD_FUN             -4
#define UNI_ERR_CMD_REGLEN          -5
#define UNI_ERR_DATA_RANGE          -6
#define UNI_ERR_MSG_TIMEOUT         -7
#define UNI_ERR_SYSTEM_MEM          -12
#define UNI_ERR_CMD_SRC_ID          -13
#define UNI_ERR_WRERROR             -14 //写入错误码为UNI_ERR_WRERROR减返回的CMD_CMAP_WRERROR代码 例如（-14 - 1）表示CMD_CMAP_WRERROR_AUTH
#define UNI_ERR_WRERROR_AUTH        -15
#define UNI_ERR_WRERROR_BUSY        -16
#define UNI_ERR_WRERROR_SIZE        -17
#define UNI_ERR_WRERROR_FORM        -18
#define UNI_ERR_IAPERROR            -19 //同UNI_ERR_WRERROR
#define UNI_ERR_IAPERROR_SIZE       -20
#define UNI_ERR_IAPERROR_ERASE      -21
#define UNI_ERR_IAPERROR_WRITEFLASH -22
#define UNI_ERR_IAPERROR_UNLOCK     -23
#define UNI_ERR_IAPERROR_INDEX      -24
#define UNI_ERR_IAPERROR_BUSY       -25
#define UNI_ERR_IAPERROR_FORM       -26
#define UNI_ERR_IAPERROR_CRC        -27
#define UNI_ERR_IAPERROR_RESET      -28
#define UNI_ERR_DEV_OPERATION       -29
#define UNI_ERR_NULL_PARSER         -30

struct uni_cmdproc_ptdata
{
    struct pt pt;
    struct pt pt_child;
    struct swtimer timer;
};

typedef struct
{
    uint8_t src_id;
    uint8_t dest_id;
    uint8_t cmd_func;
    uni_data_16_t uni_index;
    uint8_t body[UNIBUS_MAX_BODY_LEN] __ALIGNED(4); //compatible with m0
    uni_data_16_t uni_body_len;
}msg_meta_t;

typedef struct _unibus_backend
{
    unsigned int header_length;
    unsigned int checksum_length;
    unsigned int max_data_length;
    void *pt_data;
    PT_THREAD( (*read)(void *pctx, uint8_t *rbuff, int len, uint32_t *psum, int wait));
    int (*write)(void *pctx, const uint8_t *pbuff, int len);
    int (*make_send_msg)(void *pctx, msg_meta_t *p_msg_meta, uint8_t *msg, int *p_msg_len);
    int (*check_integrity)(void *pctx, uint8_t *msg, int msg_len, msg_meta_t *p_msg_meta);
    int (*receive)(void *pctx, uint8_t *req);
    int (*prepare_response)(const uint8_t *req, int *req_length);
    int (*connect)(void *pctx);
    void (*close)(void *pctx);
} unibus_backend_t;

typedef struct _unibus_t unibus_t;

struct unibus_cmd_parser
{
    struct
    {
        uint8_t cmd_func;
        int (*cmd_handler)(unibus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);
    } own_cmd_ops[UNIBUS_CMD_MAX_OP];
    int (*others_cmd_handler)(unibus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);
};

typedef struct
{
    uint8_t body[UNIBUS_MAX_MSG_LEN];
    uint8_t body_recv;
    uint8_t cmd_fun;
    uint16_t cmd_index;
    int body_len;
} msg_body_t;

struct _unibus_t
{
    void *port;       //发送设备
    int response_timeout;
    struct unibus_cmd_parser *parser;
    const unibus_backend_t *backend;
    uint8_t sub_id[UNIBUS_MAX_SUB_ID];//该handle下的所有id
    msg_body_t msg_body;
    struct pt pt_poll;
    struct pt pt_read;
    struct pt pt_recv_poll;
    uint8_t msg[UNIBUS_MAX_MSG_LEN];

    struct pt pt_receive_msg;
    struct pt pt_backend_read;
    struct swtimer swtimer_backend_read;
    struct pt pt_process;
};

int unibus_init(unibus_t *ctx);

uint32_t unibus_get_ticks(void);
/**
 * @brief 读取/写入unibus寄存器
 * 
 * @param ctx           unibus实例指针
 * @param index         读取index
 * @param read_reg_num  读取寄存器个数（每个寄存器2Byte）
 * @param reg           存储读取内容的buf
 * @return int          读取寄存器个数或错误码
 */
PT_THREAD(unibus_read_reg_poll(unibus_t *ctx, struct pt *pt, struct swtimer *read_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int read_reg_num, uint16_t *reg, int *reg_num));
PT_THREAD(unibus_write_reg_poll(unibus_t *ctx, struct pt *pt, struct swtimer *write_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int *ret_write));

/**
 * @brief 写入unibus寄存器
 * 
 * @param ctx           unibus实例指针
 * @param index         写入index
 * @param write_reg_num 写入寄存器个数（每个寄存器2Byte）
 * @param reg           写入的内容
 * @return int          写入寄存器个数或错误码
 */
int unibus_write_reg(unibus_t *ctx, uint16_t index, int write_reg_num, uint16_t *reg);

int unibus_send_read_ack(unibus_t *ctx, msg_meta_t *p_msg_meta, int reg_len, uint16_t *reg_tab);
int unibus_send_write_ack(unibus_t *ctx, msg_meta_t *p_msg_meta, uint8_t write_ret);

int unibus_received_ack(unibus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);

/**
 * @brief unibus协议处理主入口
 * 
 * @param ctx       unibus实例指针
 * @param host_id   unibus目标ID，用于过滤数据包
 * @return int      错误码
 */
PT_THREAD(unibus_recv_poll(unibus_t *ctx, int host_id));

/**
 * @brief 在 UNIBUS 设备列表中查找具有特定目标 ID 的设备。
 *
 * @param dest_id 要查找的目标 ID。
 * @param unibus_dev_table 包含 UNIBUS 设备指针的数组。
 * @param dev_num 数组中的 UNIBUS 设备指针数量。
 *
 * @return 如果找到具有匹配目标 ID 的 UNIBUS 设备指针，则返回该指针；否则返回 NULL。
 */
unibus_t *unibus_find_dest_dev(uint8_t dest_id, unibus_t *unibus_dev_table[], int dev_num);
#ifdef __cplusplus
}
#endif

#endif /* __UNIBUS_H */
