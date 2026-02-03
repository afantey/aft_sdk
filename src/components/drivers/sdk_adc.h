/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-19     rgw          first version
 */
#ifndef __SDK_ADC_H
#define __SDK_ADC_H

#include "sdk_def.h"
#include "sdk_os_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sdk_adc sdk_adc_t;

struct sdk_adc_ops
{
    sdk_err_t (*open)(sdk_adc_t *adc);
    sdk_err_t (*close)(sdk_adc_t *adc);
    sdk_err_t (*read)(sdk_adc_t *adc, uint32_t channel, uint32_t *value);
    sdk_err_t (*control)(sdk_adc_t *adc, int32_t cmd, void *args);
};

struct _sdk_adc
{
    bool is_opened;
    struct sdk_os_mutex lock;
    struct sdk_adc_ops ops;
};

sdk_err_t sdk_adc_open(sdk_adc_t *adc);
sdk_err_t sdk_adc_close(sdk_adc_t *adc);
sdk_err_t sdk_adc_read(sdk_adc_t *adc, uint32_t channel, uint32_t *value);
sdk_err_t sdk_adc_control(sdk_adc_t *adc, int32_t cmd, void *args);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_ADC_H */
