/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */
#include "modem.h"

__attribute__((weak)) int32_t modem_write(uint8_t *pbuff, uint32_t len)
{
    return 1;
}

__attribute__((weak)) int32_t modem_read(uint8_t *rbuff, uint32_t len)
{
    return 1;
}
