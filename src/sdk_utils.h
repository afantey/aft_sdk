/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         {name}       description
 */

#ifndef __SDK_UTILS_H
#define __SDK_UTILS_H

#include "sdk_config.h"
#include "sdk_def.h"


#ifdef __cplusplus
extern "C" {
#endif

#define bcd2dec(bcd) ((((bcd) >> 4) * 10) + ((bcd) & 0xf))
#define dec2bcd(dec) ((((dec) / 10) << 4) | ((dec) % 10))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


#ifdef __cplusplus
}
#endif

#endif /* __SDK_UTILS_H */