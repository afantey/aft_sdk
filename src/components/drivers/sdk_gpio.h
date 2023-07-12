/**
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-19     rgw          first version
 */

#ifndef __SDK_GPIO_H
#define __SDK_GPIO_H

#include "sdk_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sdk_gpio sdk_gpio_t;
struct sdk_gpio_ops
{
    void (*mode)(sdk_gpio_t *gpio, int pin, int mode);
    void (*write)(sdk_gpio_t *gpio, int pin, int value);
    int (*read)(sdk_gpio_t *gpio, int pin);
    int (*control)(sdk_gpio_t *gpio, int pin, int cmd, void *args);
};

struct _sdk_gpio
{
    struct sdk_gpio_ops ops;
};

#define SDK_GPIO_LOW                 0x00
#define SDK_GPIO_HIGH                0x01

#define SDK_GPIO_MODE_OUTPUT         0x00
#define SDK_GPIO_MODE_INPUT          0x01
#define SDK_GPIO_MODE_INPUT_PULLUP   0x02
#define SDK_GPIO_MODE_INPUT_PULLDOWN 0x03
#define SDK_GPIO_MODE_OUTPUT_OD      0x04

/**
 * @brief 
 * 
 * @param gpio 
 * @param pin 
 * @param mode 
 */
void sdk_gpio_mode(sdk_gpio_t *gpio, int pin, int mode);

/**
 * @brief 
 * 
 * @param gpio 
 * @param pin 
 * @param value 
 */
void sdk_gpio_write(sdk_gpio_t *gpio, int pin, int value);

/**
 * @brief 
 * 
 * @param gpio 
 * @param pin 
 * @return int 
 */
int  sdk_gpio_read(sdk_gpio_t *gpio, int pin);

/**
 * @brief 
 * 
 * @param gpio 
 * @param pin 
 * @param cmd 
 * @param args 
 * @return int 
 */

#define __GD32_PORT(port)  GPIO##port

#if defined (SOC_SERIES_GD32F4xx) || defined(SOC_SERIES_GD32L23x)
#define GD32_PIN(index, port, pin) {index, RCU_GPIO##port,      \
                                    GPIO##port, GPIO_PIN_##pin, \
                                    EXTI_SOURCE_GPIO##port,     \
                                    EXTI_SOURCE_PIN##pin}
#else
#define GD32_PIN(index, port, pin) {index, RCU_GPIO##port,        \
                                    GPIO##port, GPIO_PIN_##pin,   \
                                    GPIO_PORT_SOURCE_GPIO##port,  \
                                    GPIO_PIN_SOURCE_##pin}
#endif

int  sdk_gpio_control(sdk_gpio_t *gpio, int pin, int cmd, void *args);


#ifdef __cplusplus
}
#endif

#endif /* __SDK_GPIO_H */
