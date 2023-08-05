/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-07-26     rgw             first version
 */

#include "sdk_config.h"
#include "sdk_def.h"

double get_float_phys_from_hex(uint32_t hex, int offset, double factor)
{
    double phys = hex * factor + offset;
    return phys;
}

int get_int_phys_from_hex(uint32_t hex, int offset, double factor)
{
    int phys = (int)(hex * factor + offset);
    return phys;
}

uint16_t get_hex16_from_float_phys(double phys, int offset, double factor)
{
    uint16_t hex = (uint16_t)((phys - offset) / factor);
    return hex;
}

uint16_t get_hex16_from_int_phys(int phys, int offset, double factor)
{
    uint16_t hex = (uint16_t)((phys - offset) / factor);
    return hex;
}

uint32_t get_hex32_from_float_phys(double phys, int offset, double factor)
{
    uint32_t hex = (uint32_t)((phys - offset) / factor);
    return hex;
}


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
