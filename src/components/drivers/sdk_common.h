/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __SDK_COMMON_H
#define __SDK_COMMON_H

#include "aft_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

void sdk_hw_console_output(const char *str);
void sdk_hw_console_putc(const int ch);
void sdk_hw_us_delay(uint32_t us);
void sdk_hw_ms_delay(uint32_t ms);
void sdk_hw_interrupt_enable(void);
void sdk_hw_interrupt_disable(void);
uint32_t sdk_hw_get_systick(void);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_COMMON_H */
