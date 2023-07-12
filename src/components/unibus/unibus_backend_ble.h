/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#ifndef __UNIBUS_BACKEND_BLE_H
#define __UNIBUS_BACKEND_BLE_H

#include "sdk_config.h"
#include "sdk_def.h"

#include "unibus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _UNIBUS_BLE_HEADER_LENGTH 9
#define _UNIBUS_BLE_CHECKSUM_LENGTH 2
#define _UNIBUS_BLE_MAX_DATA_LENGTH 256

extern const unibus_backend_t backend_ble;

#ifdef __cplusplus
}
#endif

#endif /* __UNIBUS_BACKEND_BLE_H */
