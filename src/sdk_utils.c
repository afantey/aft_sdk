/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-26     rgw             first version
 */

#include "sdk_config.h"
#include "sdk_def.h"

uint16_t utils_htons(uint16_t n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

uint16_t utils_ntohs(uint16_t n)
{
    return utils_htons(n);
}

uint32_t utils_htonl(uint32_t n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

uint32_t utils_ntohl(uint32_t n)
{
    return utils_htonl(n);
}
