/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-04-18     rgw             first version
 */

#ifndef __APBUS_BACKEND_UART_H
#define __APBUS_BACKEND_UART_H

#include "sdk_config.h"
#include "sdk_def.h"

#include "apbus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _APBUS_SERIAL_HEADER_LENGTH 9
#define _APBUS_SERIAL_CHECKSUM_LENGTH 2
#define _APBUS_SERIAL_MAX_DATA_LENGTH 256

extern const apbus_backend_t backend_uart;

#ifdef __cplusplus
}
#endif

#endif /* __APBUS_BACKEND_UART_H */
