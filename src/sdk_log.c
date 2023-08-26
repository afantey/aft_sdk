/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         {name}       description
 */

#include "aft_sdk.h"
#include <time.h>
#include "sdk_common.h"

#ifndef SDK_LOG_BUF_LEN
#define SDK_LOG_BUF_LEN 256
#endif

static uint32_t id;
static void vsdklog(char *level, char *module, const char *format, va_list ap)
{
    char buf[SDK_LOG_BUF_LEN];
    int len = 0;

#if defined(SDK_LOG_USING_TIME)
    struct tm p_tm;
    time_t now = time(NULL);

    gmtime_r(&now, &p_tm);
    len = snprintf(buf, BUFLEN, "%04d-%02d-%02d %02d:%02d:%02d %s %s <%d> ", p_tm.tm_year + 1900, p_tm.tm_mon + 1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec, level, module, id);
#else
    uint32_t uptime_ms = sdk_hw_get_systick();
    unsigned int days = uptime_ms / (1000 * 60 * 60 * 24);
    unsigned int hours = (uptime_ms / (1000 * 60 * 60)) % 24;
    unsigned int minutes = (uptime_ms / (1000 * 60)) % 60;
    unsigned int seconds = (uptime_ms / 1000) % 60;
    unsigned int milliseconds = uptime_ms % 1000;
#endif
    len = snprintf(buf, SDK_LOG_BUF_LEN, "%04d %02d:%02d:%02d.%04d <%d> %s %s ", days, hours, minutes, seconds, milliseconds, id, level, module);
    // len = snprintf(buf, BUFLEN, "%s %s <%d> ", level, module, id);
    id++;
    len += vsnprintf(buf + len, SDK_LOG_BUF_LEN - len, format, ap);

    if (len > SDK_LOG_BUF_LEN - 2)
        len = SDK_LOG_BUF_LEN - 2;
    buf[len++] = '\n';
		buf[len++] = '\0';

    sdk_hw_console_output(buf);
}

void sdklog(char *level, char *module, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsdklog(level, module, format, ap);
    va_end(ap);
}

void sdklog_hexdump(uint32_t width, uint8_t *buf, uint32_t len)
{
    char temp[4];
    for(int i = 0; i < len; i++)
    {
        if(i % width == 0)
        {
            snprintf(temp, 4, "%s", "\r\n");
            sdk_hw_console_output(temp);
        }
        snprintf(temp, 4, "%02X ", buf[i]);
        sdk_hw_console_output(temp);
    }
    snprintf(temp, 4, "%s", "\r\n");
    sdk_hw_console_output(temp);
}

int fputc(int ch, FILE *f)
{
    sdk_hw_console_putc(ch);
    return ch;
}
