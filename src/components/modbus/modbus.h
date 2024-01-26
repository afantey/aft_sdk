/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-30     rgw             first version
 */

#ifndef __SDK_MODBUS_H
#define __SDK_MODBUS_H

#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MODBUS_BUF_SIZE
#define MODBUS_BUF_SIZE 200
#endif

typedef struct
{
    uint8_t   func;
    uint8_t   reg_num;
    uint16_t *reg_value;
} bMB_ReadResult_t;

typedef struct
{
    uint8_t  func;
    uint16_t reg;
    uint16_t reg_num;
} bMB_WriteResult_t;

typedef struct
{
    uint8_t type;  // 0: read     1:write
    union
    {
        bMB_ReadResult_t  r_result;
        bMB_WriteResult_t w_result;
    } result;
} bMB_SlaveDeviceData_t;

typedef void (*pMB_Send_t)(uint8_t *pbuf, uint16_t len);
typedef void (*pMB_Callback_t)(bMB_SlaveDeviceData_t *pdata);

typedef struct
{
    pMB_Send_t     f;
    pMB_Callback_t cb;
} bMB_Info_t;

typedef bMB_Info_t bModbusInstance_t;


#define L2B_B2L_16b(n) ((((n)&0xff) << 8) | (((n)&0xff00) >> 8))

int bMB_ReadRegs(bModbusInstance_t *pModbusInstance, uint8_t addr, uint8_t func, uint16_t reg,
                 uint16_t num);
int bMB_WriteRegs(bModbusInstance_t *pModbusInstance, uint8_t addr, uint8_t func, uint16_t reg,
                  uint16_t num, uint16_t *reg_value);
int bMB_FeedReceivedData(bModbusInstance_t *pModbusInstance, uint8_t *pbuf, uint16_t len);
uint16_t _bMBCRC16(uint8_t *pucFrame, uint16_t usLen);
#ifdef __cplusplus
}
#endif

#endif /* __SDK_MODBUS_H */
