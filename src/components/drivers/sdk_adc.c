/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-04     rgw          first version
 */

#include "sdk_adc.h"

sdk_err_t sdk_adc_open(sdk_adc_t *adc)
{
    sdk_err_t ret = SDK_OK;

    if (adc == NULL || adc->ops.open == NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = adc->ops.open(adc);
    if (ret == SDK_OK)
    {
        adc->is_opened = true;
    }

    return ret;
}

sdk_err_t sdk_adc_close(sdk_adc_t *adc)
{
    sdk_err_t ret = SDK_OK;

    if (adc == NULL || adc->ops.close== NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = adc->ops.close(adc);
    if (ret == SDK_OK)
    {
        adc->is_opened = false;
    }

    return ret;
}

sdk_err_t sdk_adc_read(sdk_adc_t *adc, uint32_t channel, uint32_t *value)
{
    sdk_err_t ret = SDK_OK;
    uint32_t val = 0;

    if (adc == NULL || adc->ops.read == NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = adc->ops.read(adc, channel, &val);
    if (ret == SDK_OK)
    {
        *value = val;
    }

    return ret;
}

sdk_err_t sdk_adc_control(sdk_adc_t *adc, int32_t cmd, void *args)
{
    sdk_err_t ret = SDK_OK;

    if(adc == NULL || adc->ops.control == NULL)
    {
        return -SDK_E_INVALID;
    }

    ret = adc->ops.control(adc, cmd, args);

    return ret;
}
