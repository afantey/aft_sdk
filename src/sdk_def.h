/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

//SDK 内部不需要变动的宏定义
#ifndef __SDK_DEF_H
#define __SDK_DEF_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SDK version information */
#define SDK_VERSION_MAJOR  0 /**< major version number */
#define SDK_VERSION_MINOR  0 /**< minor version number */
#define SDK_VERSION_REVISE 1 /**< revise version number */

/* SDK version */
#define SDK_VERSION SDK_VERSION_CHECK(SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_REVISE)

/* e.g. #if (RTTHREAD_VERSION >= SDK_VERSION_CHECK(4, 1, 0) */
#define SDK_VERSION_CHECK(major, minor, revise) ((major * 10000) + \
                                                 (minor * 100) + revise)

/* Define one bit mask */
#ifndef BIT0
#define BIT0     (0x00000001UL)       ///< Bit 0 mask of an 32 bit integer
#define BIT1     (0x00000002UL)       ///< Bit 1 mask of an 32 bit integer
#define BIT2     (0x00000004UL)       ///< Bit 2 mask of an 32 bit integer
#define BIT3     (0x00000008UL)       ///< Bit 3 mask of an 32 bit integer
#define BIT4     (0x00000010UL)       ///< Bit 4 mask of an 32 bit integer
#define BIT5     (0x00000020UL)       ///< Bit 5 mask of an 32 bit integer
#define BIT6     (0x00000040UL)       ///< Bit 6 mask of an 32 bit integer
#define BIT7     (0x00000080UL)       ///< Bit 7 mask of an 32 bit integer
#define BIT8     (0x00000100UL)       ///< Bit 8 mask of an 32 bit integer
#define BIT9     (0x00000200UL)       ///< Bit 9 mask of an 32 bit integer
#define BIT10    (0x00000400UL)       ///< Bit 10 mask of an 32 bit integer
#define BIT11    (0x00000800UL)       ///< Bit 11 mask of an 32 bit integer
#define BIT12    (0x00001000UL)       ///< Bit 12 mask of an 32 bit integer
#define BIT13    (0x00002000UL)       ///< Bit 13 mask of an 32 bit integer
#define BIT14    (0x00004000UL)       ///< Bit 14 mask of an 32 bit integer
#define BIT15    (0x00008000UL)       ///< Bit 15 mask of an 32 bit integer
#define BIT16    (0x00010000UL)       ///< Bit 16 mask of an 32 bit integer
#define BIT17    (0x00020000UL)       ///< Bit 17 mask of an 32 bit integer
#define BIT18    (0x00040000UL)       ///< Bit 18 mask of an 32 bit integer
#define BIT19    (0x00080000UL)       ///< Bit 19 mask of an 32 bit integer
#define BIT20    (0x00100000UL)       ///< Bit 20 mask of an 32 bit integer
#define BIT21    (0x00200000UL)       ///< Bit 21 mask of an 32 bit integer
#define BIT22    (0x00400000UL)       ///< Bit 22 mask of an 32 bit integer
#define BIT23    (0x00800000UL)       ///< Bit 23 mask of an 32 bit integer
#define BIT24    (0x01000000UL)       ///< Bit 24 mask of an 32 bit integer
#define BIT25    (0x02000000UL)       ///< Bit 25 mask of an 32 bit integer
#define BIT26    (0x04000000UL)       ///< Bit 26 mask of an 32 bit integer
#define BIT27    (0x08000000UL)       ///< Bit 27 mask of an 32 bit integer
#define BIT28    (0x10000000UL)       ///< Bit 28 mask of an 32 bit integer
#define BIT29    (0x20000000UL)       ///< Bit 29 mask of an 32 bit integer
#define BIT30    (0x40000000UL)       ///< Bit 30 mask of an 32 bit integer
#define BIT31    (0x80000000UL)       ///< Bit 31 mask of an 32 bit integer
#endif

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int32_t sdk_err_t;

typedef union {
    uint16_t data_16;
    uint8_t data_8[2];
} uni_data_16_t;

typedef union
{
   uint32_t data_32;
   uint16_t data_16[2];
   uint8_t data_8[4];
}uni_data_32_t;

/* error code definitions */
#define SDK_OK        0 /**< no error */
#define SDK_ERROR     1 /**< generic error */
#define SDK_E_TIMEOUT 2 /**< timeout */
#define SDK_E_MEMORY  3 /**< No memory */
#define SDK_E_BUSY    4 /**< Busy */
#define SDK_E_INVALID 5 /**< Invalid argument */

/* driver definitions */
#define SDK_CONTROL_UART_DISABLE_INT      0 /**< Disable interrupt */
#define SDK_CONTROL_UART_ENABLE_INT       1 /**< Enable interrupt */
#define SDK_CONTROL_UART_ENABLE_DMA       2 /**< Enable DMA */
#define SDK_CONTROL_UART_INT_IDLE_ENABLE  3 // recv idle interrupt
#define SDK_CONTROL_UART_INT_IDLE_DISABLE 4
#define SDK_CONTROL_UART_INT_RTO_ENABLE   5 // recv timeout interrupt
#define SDK_CONTROL_UART_INT_RTO_DISABLE  6
#define SDK_CONTROL_UART_GET_DMA_CNT      7
#define SDK_CONTROL_UART_ENABLE_RX        8
#define SDK_CONTROL_UART_DISABLE_RX       9

#define SDK_CONTROL_RTC_GET_TIME   0 /**< Get time */
#define SDK_CONTROL_RTC_SET_TIME   1 /**< Set time */
#define SDK_CONTROL_RTC_GET_ALARM  2 /**< Get alarm */
#define SDK_CONTROL_RTC_SET_ALARM  3 /**< Set alarm */
#define SDK_CONTROL_RTC_SET_WAKEUP 4

#define SDK_DRIVER_WDT_GET_TIMEOUT    (1) /* get timeout(in seconds) */
#define SDK_DRIVER_WDT_SET_TIMEOUT    (2) /* set timeout(in seconds) */
#define SDK_DRIVER_WDT_GET_TIMELEFT   (3) /* get the left time before reboot(in seconds) */
#define SDK_DRIVER_WDT_KEEPALIVE      (4) /* refresh watchdog */
#define SDK_DRIVER_WDT_START          (5) /* start watchdog */
#define SDK_DRIVER_WDT_STOP           (6) /* stop watchdog */

#ifdef __cplusplus
}
#endif

#endif /* __SDK_DEF_H */
