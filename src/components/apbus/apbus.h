/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-04-18     rgw             first version
 */

#ifndef __APBUS_H
#define __APBUS_H

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
 * APBUS配置参数
 * 
 ******************************************************/
#define APBUS_MAX_MSG_LEN  280
#define APBUS_MAX_BODY_LEN 256
#define APBUS_MAX_SUB_ID   4  //一个端口下最多可能会有多少个设备

#define APBUS_CMD_MAX_OP 16


/******************************************************
 * 
 * 通用命令字，在设备poll头文件中，可能有专用控制字
 * 
 ******************************************************/
// 控制表访问指令
#define CMD_CMAP_RD         0x03    // 读控制表数据
#define CMD_CMAP_WR         0x10    // 写控制表数据，需要应答。
#define CMD_CMAP_WR_NR      0x11    // 写控制表数据，无应答
#define CMD_CMAP_ACK_RD     0x84    // 对读取指令的响应包
#define CMD_CMAP_ACK_WR     0x91    // 对写指令的响应包
// 数据下载相关指令
#define CMD_FILE_NEW                   0x13 // 新建文件
#define CMD_FILE_WRITE                 0x15 // 写文件
#define CMD_FILE_NEW_ACK               0x94 // 新建文件应答帧
#define CMD_FILE_WRITE_ACK             0x96 // 写文件应答帧

/******************************************************
 * 
 * 协议操作失败原因
 * 
 ******************************************************/

#define CMD_CMAP_WRERROR_FORM 0x03 //写入数据格式错误
// 代码	名称	含义
// 01	非法功能	接收到的功能码不允许。
// 02	非法数据地址	接收到的数据地址或文件ID不允许。
// 03	非法数据值	接收到的值不允许。
// 04	设备故障	在执行请求操作时发生不可恢复的错误。
// 05	确认	已接受请求，正在处理，但需要较长的持续时间。
// 06	设备忙	正在处理长持续时间的程序命令。
// 08	存储错误	写寄存器或者文件存储数据失败
// 0A	不可用路径	无法处理的目的ID
// 0B	目标设备响应失败	目的设备相应超时。
#define CMD_CMAP_WRERROR_FUN           0x01
#define CMD_CMAP_WRERROR_ADDR          0x02
#define CMD_CMAP_WRERROR_VALUE         0x03
#define CMD_CMAP_WRERROR_DEV           0x04
#define CMD_CMAP_WRERROR_CONFIRM       0x05
#define CMD_CMAP_WRERROR_BUSY          0x06
#define CMD_CMAP_WRERROR_WRITE         0x08
#define CMD_CMAP_WRERROR_ID            0x0A
#define CMD_CMAP_WRERROR_TIMEOUT       0x0B

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
 * APBUS错误代码
 * 
 ******************************************************/
#define APBUS_ERR_NO                   0
#define APBUS_ERR_PROTO_BROKEN        -1
#define APBUS_ERR_PROTO_CHECK         -2
#define APBUS_ERR_CMD_INDEX           -3
#define APBUS_ERR_CMD_FUN             -4
#define APBUS_ERR_CMD_REGLEN          -5
#define APBUS_ERR_DATA_RANGE          -6
#define APBUS_ERR_MSG_TIMEOUT         -7
#define APBUS_ERR_SYSTEM_MEM          -12
#define APBUS_ERR_CMD_SRC_ID          -13
#define APBUS_ERR_WRERROR             -14 //写入错误码为APBUS_ERR_WRERROR减返回的CMD_CMAP_WRERROR代码 例如（-14 - 1）表示CMD_CMAP_WRERROR_AUTH
#define APBUS_ERR_WRERROR_AUTH        -15
#define APBUS_ERR_WRERROR_BUSY        -16
#define APBUS_ERR_WRERROR_SIZE        -17
#define APBUS_ERR_WRERROR_FORM        -18
#define APBUS_ERR_IAPERROR            -19 //同APBUS_ERR_WRERROR
#define APBUS_ERR_IAPERROR_SIZE       -20
#define APBUS_ERR_IAPERROR_ERASE      -21
#define APBUS_ERR_IAPERROR_WRITEFLASH -22
#define APBUS_ERR_IAPERROR_UNLOCK     -23
#define APBUS_ERR_IAPERROR_INDEX      -24
#define APBUS_ERR_IAPERROR_BUSY       -25
#define APBUS_ERR_IAPERROR_FORM       -26
#define APBUS_ERR_IAPERROR_CRC        -27
#define APBUS_ERR_IAPERROR_RESET      -28
#define APBUS_ERR_DEV_OPERATION       -29
#define APBUS_ERR_NULL_PARSER         -30

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
    uint8_t body[APBUS_MAX_BODY_LEN]; //compatible with m0
    uni_data_16_t uni_body_len;
}msg_meta_t;

typedef struct _apbus_backend
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
} apbus_backend_t;

typedef struct _apbus_t apbus_t;

struct apbus_cmd_parser
{
    struct
    {
        uint8_t cmd_func;
        int (*cmd_handler)(apbus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);
    } own_cmd_ops[APBUS_CMD_MAX_OP];
    int (*others_cmd_handler)(apbus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);
};

typedef struct
{
    uint8_t body[APBUS_MAX_MSG_LEN];
    uint8_t body_recv;
    uint8_t cmd_fun;
    uint16_t cmd_index;
    int body_len;
} msg_body_t;

struct _apbus_t
{
    void *port;       //发送设备
    int response_timeout;
    struct apbus_cmd_parser *parser;
    const apbus_backend_t *backend;
    uint8_t sub_id[APBUS_MAX_SUB_ID];//该handle下的所有id
    msg_body_t msg_body;
    struct pt pt_poll;
    struct pt pt_read;
    struct pt pt_recv_poll;
    uint8_t msg[APBUS_MAX_MSG_LEN];

    struct pt pt_receive_msg;
    struct pt pt_backend_read;
    struct swtimer swtimer_backend_read;
    struct pt pt_process;
};

int apbus_init(apbus_t *ctx);

uint32_t apbus_get_ticks(void);
/**
 * @brief 读取/写入apbus寄存器
 * 
 * @param ctx           apbus实例指针
 * @param index         读取index
 * @param read_reg_num  读取寄存器个数（每个寄存器2Byte）
 * @param reg           存储读取内容的buf
 * @return int          读取寄存器个数或错误码
 */
PT_THREAD(apbus_read_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *read_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int read_reg_num, uint16_t *reg, int *reg_num));
PT_THREAD(apbus_write_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *write_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int *ret_write));
PT_THREAD(apbus_write_NR_reg_poll(apbus_t *ctx, struct pt *pt, struct swtimer *write_reg_timer, uint32_t response_timeout, uint8_t src_id, uint8_t dest_id, uint16_t index, int write_reg_num, uint16_t *reg, int *ret_write));

/**
 * @brief 写入apbus寄存器
 * 
 * @param ctx           apbus实例指针
 * @param index         写入index
 * @param write_reg_num 写入寄存器个数（每个寄存器2Byte）
 * @param reg           写入的内容
 * @return int          写入寄存器个数或错误码
 */
int apbus_write_reg(apbus_t *ctx, uint16_t index, int write_reg_num, uint16_t *reg);

int apbus_send_read_ack(apbus_t *ctx, msg_meta_t *p_msg_meta, int reg_len, uint16_t *reg_tab);
int apbus_send_write_ack(apbus_t *ctx, msg_meta_t *p_msg_meta, uint8_t write_ret);

int apbus_received_ack(apbus_t *ctx, uint8_t *msg, msg_meta_t *p_msg_meta);

/**
 * @brief apbus协议处理主入口
 * 
 * @param ctx       apbus实例指针
 * @param host_id   apbus目标ID，用于过滤数据包
 * @return int      错误码
 */
PT_THREAD(apbus_recv_poll(apbus_t *ctx, int host_id));

/**
 * @brief 在 APBUS 设备列表中查找具有特定目标 ID 的设备。
 *
 * @param dest_id 要查找的目标 ID。
 * @param apbus_dev_table 包含 APBUS 设备指针的数组。
 * @param dev_num 数组中的 APBUS 设备指针数量。
 *
 * @return 如果找到具有匹配目标 ID 的 APBUS 设备指针，则返回该指针；否则返回 NULL。
 */
apbus_t *apbus_find_dest_dev(uint8_t dest_id, apbus_t *apbus_dev_table[], int dev_num);
#ifdef __cplusplus
}
#endif

#endif /* __APBUS_H */
