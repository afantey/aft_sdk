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

/**
 * @brief 计算线性回归的斜率
 * @param x x值数组
 * @param y y值数组
 * @param data_size 数据数组大小
 * @return 斜率值
 */
double calculate_slope(double x[], double y[], int data_size)
{
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;

    // 计算累加和
    for (int i = 0; i < data_size; i++)
    {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    // 计算斜率的分子和分母
    double numerator = (data_size * sum_xy) - (sum_x * sum_y);
    double denominator = (data_size * sum_x2) - (sum_x * sum_x);

    // 计算斜率并返回
    return numerator / denominator;
}

/**
 * @brief 判断数据的离散程度是否在指定范围内
 * @param data 数据数组
 * @param data_size 数据数组大小
 * @param range 离散程度范围
 * @param slope_limit 斜率限制
 * @return 如果在范围内返回1，否则返回0
 */
int is_dispersion_within_range(double data[], int data_size, double range, double slope_limit)
{
    // 构造 x 数组，假设 x 是 [0, 1, 2, ...]
    double x[data_size];
    for (int i = 0; i < data_size; i++)
    {
        x[i] = (double)i;
    }

    // 计算数据的斜率
    double slope = calculate_slope(x, data, data_size);

    double sum = 0.0;
    for (int i = 0; i < data_size; i++)
    {
        sum += data[i];
    }
    double mean = sum / data_size;

    double squared_diff_sum = 0.0;
    for (int i = 0; i < data_size; i++)
    {
        double diff = data[i] - mean;
        squared_diff_sum += diff * diff;
    }
    double variance = squared_diff_sum / data_size;
    double standard_deviation = sqrtf(variance);

    // 判断数据的离散程度和斜率是否在限制范围内
    return standard_deviation <= range && fabs(slope) <= slope_limit;
}

int32_t utils_hex2str(uint8_t *input, int input_len, char *output, int output_len)
{
    int i = 0, j = 0;
    uint8_t tmp = 0;

    if (output_len < input_len * 2 + 1) {
        return -1;
    }

    for (i = 0; i < input_len; i++) {
        tmp = input[i] >> 4;
        if (tmp <= 9) {
            output[j] = tmp + '0';
        } else if (tmp >= 10 && tmp <= 15) {
            output[j] = tmp - 10 + 'A';
        } else {
            return -1;
        }
        j++;

        tmp = input[i] & 0x0f;
        if (tmp <= 9) {
            output[j] = tmp + '0';
        } else if (tmp >= 10 && tmp <= 15) {
            output[j] = tmp - 10 + 'A';
        } else {
            return -1;
        }
        j++;
    }
    output[j] = '\0';
    return 0;
}

int32_t utils_str2hex(char *input, int input_len, uint8_t *output, int output_len)
{
    int i = 0, j = 0;
    uint8_t tmp = 0;

    if (output_len < input_len / 2) {
        return -1;
    }

    for (i = 0; i < input_len; i++) {
        if (input[i] >= '0' && input[i] <= '9') {
            tmp = input[i] - '0';
        } else if (input[i] >= 'A' && input[i] <= 'F') {
            tmp = input[i] - 'A' + 10;
        } else if (input[i] >= 'a' && input[i] <= 'f') {
            tmp = input[i] - 'a' + 10;
        } else {
            return -1;
        }

        if (i % 2 == 0) {
            output[j] = tmp << 4;
        } else {
            output[j] |= tmp;
            j++;
        }
    }
    return 0;
}
