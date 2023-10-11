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

double get_float_phys_from_hex(uint32_t hex, int offset, double factor);
int get_int_phys_from_hex(uint32_t hex, int offset, double factor);
uint16_t get_hex16_from_float_phys(double phys, int offset, double factor);
uint16_t get_hex16_from_int_phys(int phys, int offset, double factor);
uint32_t get_hex32_from_float_phys(double phys, int offset, double factor);
double calculate_slope(double x[], double y[], int data_size);
int is_dispersion_within_range(double data[], int data_size, double range, double slope_limit);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_UTILS_H */
