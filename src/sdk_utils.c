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

/**
 * @brief �������Իع��б��
 * @param x xֵ����
 * @param y yֵ����
 * @param data_size ���������С
 * @return б��ֵ
 */
double calculate_slope(double x[], double y[], int data_size)
{
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;

    // �����ۼӺ�
    for (int i = 0; i < data_size; i++)
    {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    // ����б�ʵķ��Ӻͷ�ĸ
    double numerator = (data_size * sum_xy) - (sum_x * sum_y);
    double denominator = (data_size * sum_x2) - (sum_x * sum_x);

    // ����б�ʲ�����
    return numerator / denominator;
}

/**
 * @brief �ж����ݵ���ɢ�̶��Ƿ���ָ����Χ��
 * @param data ��������
 * @param data_size ���������С
 * @param range ��ɢ�̶ȷ�Χ
 * @param slope_limit б������
 * @return ����ڷ�Χ�ڷ���1�����򷵻�0
 */
int is_dispersion_within_range(double data[], int data_size, double range, double slope_limit)
{
    // ���� x ���飬���� x �� [0, 1, 2, ...]
    double x[data_size];
    for (int i = 0; i < data_size; i++)
    {
        x[i] = (double)i;
    }

    // �������ݵ�б��
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

    // �ж����ݵ���ɢ�̶Ⱥ�б���Ƿ������Ʒ�Χ��
    return standard_deviation <= range && fabs(slope) <= slope_limit;
}
