/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-21     rgw          first version
 */

#include "aft_sdk.h"

void sdk_pin_mode(sdk_gpio_t *gpio, int pin, int mode)
{
    gpio->ops.mode(gpio, pin, mode);
}

void sdk_gpio_write(sdk_gpio_t *gpio, int pin, int value)
{
    gpio->ops.write(gpio, pin, value);
}

int sdk_gpio_read(sdk_gpio_t *gpio, int pin)
{
    return gpio->ops.read(gpio, pin);
}
