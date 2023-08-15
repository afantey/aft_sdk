/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#ifndef __SDK_LOG_H
#define __SDK_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

void sdklog(char *level, char *module, const char *format, ...);
void sdklog_hexdump(uint32_t width, uint8_t *buf, uint32_t len);
int sdk_printf(const char *format, ...);

/* DEBUG level */
#define DBG_ERROR           0
#define DBG_WARNING         1
#define DBG_INFO            2
#define DBG_LOG             3

#ifdef DBG_TAG
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME    DBG_TAG
#endif
#else
/* compatible with old version */
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME    "DBG"
#endif
#endif /* DBG_TAG */

#ifdef DBG_LVL
#ifndef DBG_LEVEL
#define DBG_LEVEL         DBG_LVL
#endif
#else
/* compatible with old version */
#ifndef DBG_LEVEL
#define DBG_LEVEL         DBG_WARNING
#endif
#endif /* DBG_LVL */


#if (DBG_LEVEL >= DBG_LOG)
#define LOG_D(fmt, ...)      sdklog("DEBUG", DBG_SECTION_NAME, "%s():%d, " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (DBG_LEVEL >= DBG_INFO)
#define LOG_I(fmt, ...)      sdklog("INFO", DBG_SECTION_NAME, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (DBG_LEVEL >= DBG_WARNING)
#define LOG_W(fmt, ...)      sdklog("WARN", DBG_SECTION_NAME, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (DBG_LEVEL >= DBG_ERROR)
#define LOG_E(fmt, ...)      sdklog("ERROR", DBG_SECTION_NAME, "%s():%d, " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif

#define LOG_HEX(width, buf, size) sdklog_hexdump(width, buf, size)

#define sdk_printf(...) printf(__VA_ARGS__)

#define LOG_RAW(...)    sdk_printf(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __SDK_LOG_H */
