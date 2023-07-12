/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#include "tm1668.h"

#define TM1668_DISPLAY_BUFFER_SIZE 14
#define TM1668_DISPLAY_MAX_DIGITS  2

struct tm1668_character characters[] = {
	TM1668_SEG_DIGIT,
};

#define TM1668_CMD_DISPLAY_MODE(mode) \
    (0x00 |                           \
     (mode & 0x3))

#define TM1668_CMD_DATA(test_not_normal_mode, display_address_increment, \
                        read_not_write)                                  \
    (0x40 |                                                              \
     (test_not_normal_mode ? 0x4 : 0x0) |                                \
     (display_address_increment ? 0x0 : 0x1) |                           \
     (read_not_write ? 0x2 : 0x0))

#define TM1668_CMD_ADDRESS(address) \
    (0xc0 |                         \
     (address & 0x0f))

#define TM1668_CMD_DISPLAY_CONTROL(on, pulse_width) \
    (0x80 |                                         \
     (on ? 0x8 : 0x0) |                             \
     (pulse_width & 0x7))


static const struct device *dev_p0;
static const struct device *dev_p5;

static inline int gpio_get_value(uint32_t gpio)
{
	int pin_val;

	if(gpio == 5)
	{
		pin_val = gpio_pin_get(dev_p0, 5);
		return pin_val;
	}
	else
	{
		return -1;
	}
}

static inline void gpio_set_value(uint32_t gpio, int value)
{
	if(gpio == 4)
	{
        gpio_pin_set(dev_p0, 4, value);
    }
	if(gpio == 5)
	{
        gpio_pin_set(dev_p0, 5, value);
    }
	if(gpio == 56)
	{
        gpio_pin_set(dev_p5, 6, value);
	}
}

static inline void gpio_direction_input(uint32_t gpio)
{
	int ret;
	if(gpio == 5)
	{
		ret = gpio_pin_configure(dev_p0, 5, GPIO_INPUT);
	}
	else
	{
		return;
	}

}

static inline void gpio_direction_output(uint32_t gpio, uint32_t level)
{
	int ret;
	if(gpio == 5)
	{
		if(level == 1)
		{
            ret = gpio_pin_configure(dev_p0, 5, GPIO_OUTPUT_ACTIVE | GPIO_OPEN_DRAIN);
        }
		else
		{
            ret = gpio_pin_configure(dev_p0, 5, GPIO_OUTPUT_INACTIVE | GPIO_OPEN_DRAIN);
        }
	}
	else
	{
		return;
	}
}

static void udelay(unsigned long usecs)
{
	//sdk_hw_us_delay(usecs);
}

void testgpio(void)
{
	gpio_pin_set(dev_p0, 5, 0);
	udelay(1);
	gpio_pin_set(dev_p0, 5, 1);
	udelay(1);
}
static int tm1668_gpio_init(struct tm1668_chip *chip)
{
	int ret;
	chip->gpio_dio = 5;
	chip->gpio_sclk = 4;
	chip->gpio_stb = 56;
	
	dev_p0 = device_get_binding("GPIO_P0");
	dev_p5 = device_get_binding("GPIO_P5");
	ret = gpio_pin_configure(dev_p5, 6, GPIO_OUTPUT_ACTIVE | GPIO_OPEN_DRAIN);
    ret = gpio_pin_configure(dev_p0, 4, GPIO_OUTPUT_ACTIVE | GPIO_OPEN_DRAIN);
    ret = gpio_pin_configure(dev_p0, 5, GPIO_OUTPUT_ACTIVE | GPIO_OPEN_DRAIN);
	// while(1)
	// {
	// 	testgpio();
	// }
    // ret = gpio_pin_configure(dev_p1, 6, GPIO_OUTPUT_ACTIVE | GPIO_PUSH_PULL);
	// gpio_pin_set(dev_p5, 6, 1);
	// gpio_pin_set(dev_p0, 4, 1);
	// gpio_pin_set(dev_p0, 5, 1);

	return ret;
}


/* Serial interface */

static void tm1668_writeb(struct tm1668_chip *chip, uint8_t byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		gpio_set_value(chip->gpio_dio, byte & 0x1);
		gpio_set_value(chip->gpio_sclk, 0);
		udelay(1);
		gpio_set_value(chip->gpio_sclk, 1);
		udelay(1);

		byte >>= 1;
	}
}

// static void tm1668_readb(struct tm1668_chip *chip, uint8_t *byte)
// {
//     int i;

//     *byte = 0;
//     for (i = 0; i < 8; i++)
//     {
//         gpio_set_value(chip->gpio_sclk, 0);
//         udelay(1);
//         *byte |= gpio_get_value(chip->gpio_dio);
//         gpio_set_value(chip->gpio_sclk, 1);
//         udelay(1);

//         *byte <<= 1;
//     }
// }

static void tm1668_send(struct tm1668_chip *chip, uint8_t command,
		const void *buf, int len)
{
	const uint8_t *data = buf;
	int i;

	// BUG_ON(len > TM1668_DISPLAY_BUFFER_SIZE);

	gpio_set_value(chip->gpio_stb, 0);
	udelay(1);
	tm1668_writeb(chip, command);
	for (i = 0; i < len; i++)
		tm1668_writeb(chip, *data++);
	udelay(1);
	gpio_set_value(chip->gpio_stb, 1);
	udelay(2);
}

// static void tm1668_recv(struct tm1668_chip *chip, uint8_t command,
//                         void *buf, int len)
// {
//     uint8_t *data = buf;
//     int i;

//     gpio_set_value(chip->gpio_stb, 0);
//     udelay(1);
//     tm1668_writeb(chip, command);
//     udelay(1);
//     gpio_direction_input(chip->gpio_dio);
//     for (i = 0; i < len; i++)
//         tm1668_readb(chip, data++);
//     gpio_direction_output(chip->gpio_dio, 1);
//     udelay(1);
//     gpio_set_value(chip->gpio_stb, 1);
//     udelay(2);
// }

/* Display control & sysfs interface */

void tm1668_clear(struct tm1668_chip *chip)
{
	uint8_t data[TM1668_DISPLAY_BUFFER_SIZE] = { 0, };

	tm1668_send(chip, TM1668_CMD_DATA(0, 1, 0), NULL, 0);
	tm1668_send(chip, TM1668_CMD_ADDRESS(0xC), data, sizeof(data));
}

void tm1668_set_brightness(struct tm1668_chip *chip, int brightness)
{
	int enabled = brightness > 0;

	if (brightness < 0)
		brightness = 0;
	else if (brightness > 8)
		brightness = 8;
	chip->brightness = brightness;

	if (enabled)
		brightness--;

	tm1668_send(chip, TM1668_CMD_DISPLAY_CONTROL(enabled, brightness),
			NULL, 0);
}

void tm1668_print_icon(struct tm1668_chip *chip, const uint16_t character, int display)
{
	uint8_t address = 0;
    uint16_t data[1] = {0};
    for (int i = 0; i < chip->characters_num; i++)
	{
        if (chip->characters[i].character == character)
        {
			address = chip->characters[i].address;
			if(display == 1)
     	    	chip->reg[address].value |= chip->characters[i].value;
			else
				chip->reg[address].value &= ((~chip->characters[i].value) & 0xFFF);
			data[0] = chip->reg[address].value ;
            break;
        }
    }

    tm1668_send(chip, TM1668_CMD_DATA(0, 1, 0), NULL, 0);
	tm1668_send(chip, TM1668_CMD_ADDRESS(address), data, sizeof(data));
}

// void tm1668_print(struct tm1668_chip *chip, const uint16_t character)
// {
// 	uint8_t address = 0;
//     uint16_t data[1] = {0};
//     for (int i = 0; i < chip->characters_num; i++)
// 	{
//         if (chip->characters[i].character == character)
//         {
// 			address = chip->characters[i].address;
// 			data[0] = chip->characters[i].value;
//             break;
//         }
//     }

//     tm1668_send(chip, TM1668_CMD_DATA(0, 1, 0), NULL, 0);
// 	tm1668_send(chip, TM1668_CMD_ADDRESS(address), data, sizeof(data));
// }

void tm1668_print_num(struct tm1668_chip *chip, const uint16_t character)
{
	uint8_t address = 0;
    uint16_t data[1] = {0};
    for (int i = 0; i < chip->characters_num; i++)
	{
        if (chip->characters[i].character == character)
        {
			address = chip->characters[i].address;
			data[0] = (chip->characters[i].value | (chip->reg[address].value & 0x80));
            break;
        }
    }

    tm1668_send(chip, TM1668_CMD_DATA(0, 1, 0), NULL, 0);
	tm1668_send(chip, TM1668_CMD_ADDRESS(address), data, sizeof(data));
}

void tm1668_print_speed(struct tm1668_chip *chip, uint16_t speed)
{
	uint8_t speed_l = speed % 10;
	uint8_t speed_h = speed / 10;

	if(speed_h > 9)
	{
		speed_l = 9;
		speed_h = 9;
	}

	tm1668_print_num(chip, speed_h);
	tm1668_print_num(chip, speed_l+10);
}

#include "unibus_poll_mcu.h"
void tm1668_print_mode(struct tm1668_chip *chip, uint16_t mode)
{
    switch (mode)
    {
    case WORKMODE_WALK_ASSIST:
        tm1668_print_icon(chip, 42, 0);
        tm1668_print_icon(chip, 41, 0);
        tm1668_print_icon(chip, 43, 1);
        break;
    case WORKMODE_ECO:
        break;
    case WORKMODE_NORMAL:
		tm1668_print_icon(chip, 43, 0);
		tm1668_print_icon(chip, 41, 0);
		tm1668_print_icon(chip, 42, 1);
        break;
    case WORKMODE_SPORT:
		tm1668_print_icon(chip, 43, 0);
		tm1668_print_icon(chip, 42, 0);
		tm1668_print_icon(chip, 41, 1);
        break;
    default:
        break;
    }
}

void tm1668_print_battery(struct tm1668_chip *chip, uint16_t battery_precent)
{
	uint8_t btc = battery_precent / 15;

	if(btc > 5)
		btc = 5;

	switch (btc)
	{
	case 5:
		tm1668_print_icon(chip, 54, 0); //1 red
		tm1668_print_icon(chip, 52, 0); //2 red
		tm1668_print_icon(chip, 55, 1); //1 white
		tm1668_print_icon(chip, 53, 1); //2 white
		tm1668_print_icon(chip, 59, 1); //3 white
		tm1668_print_icon(chip, 58, 1); //4 white
		tm1668_print_icon(chip, 57, 1); //5 white
        break;
    case 4:
		tm1668_print_icon(chip, 54, 0); //1 red
		tm1668_print_icon(chip, 52, 0); //2 red
		tm1668_print_icon(chip, 57, 0); //5 white
		tm1668_print_icon(chip, 55, 1); //1 white
		tm1668_print_icon(chip, 53, 1); //2 white
		tm1668_print_icon(chip, 59, 1); //3 white
		tm1668_print_icon(chip, 58, 1); //4 white
        break;
    case 3:
		tm1668_print_icon(chip, 54, 0); //1 red
		tm1668_print_icon(chip, 52, 0); //2 red
		tm1668_print_icon(chip, 58, 0); //4 white
		tm1668_print_icon(chip, 57, 0); //5 white
		tm1668_print_icon(chip, 55, 1); //1 white
		tm1668_print_icon(chip, 53, 1); //2 white
		tm1668_print_icon(chip, 59, 1); //3 white
        break;
    case 2:
		tm1668_print_icon(chip, 57, 0); //5 white
		tm1668_print_icon(chip, 58, 0); //4 white
		tm1668_print_icon(chip, 59, 0); //3 white
		tm1668_print_icon(chip, 53, 0); //2 white
		tm1668_print_icon(chip, 55, 0); //1 white
		tm1668_print_icon(chip, 54, 1); //1 red
		tm1668_print_icon(chip, 52, 1); //2 red
        break;
    case 1:
		tm1668_print_icon(chip, 57, 0); //5 white
		tm1668_print_icon(chip, 58, 0); //4 white
		tm1668_print_icon(chip, 59, 0); //3 white
		tm1668_print_icon(chip, 53, 0); //2 white
		tm1668_print_icon(chip, 55, 0); //1 white
		tm1668_print_icon(chip, 52, 0); //2 red
		tm1668_print_icon(chip, 54, 1); //1 red
        break;
    case 0:
		tm1668_print_icon(chip, 54, 0); //1 red
		tm1668_print_icon(chip, 52, 0); //2 red
		tm1668_print_icon(chip, 57, 0); //5 white
		tm1668_print_icon(chip, 58, 0); //4 white
		tm1668_print_icon(chip, 59, 0); //3 white
		tm1668_print_icon(chip, 53, 0); //2 white
		tm1668_print_icon(chip, 55, 0); //1 white
        break;
    default:
        break;
    }
}

/* Driver routines */

int tm1668_init(struct tm1668_chip *chip)
{
	int err = 0;

	/* Allocate GPIO lines... */

	tm1668_gpio_init(chip);

	/* Initialize the chip */

	tm1668_send(chip, TM1668_CMD_DISPLAY_MODE(tm1668_mode_7_digits_10_segments), NULL, 0);

	/* Initialize display interface */

	tm1668_clear(chip);
	tm1668_set_brightness(chip, 1);
    chip->characters_num = sizeof(characters) / sizeof(struct tm1668_character);
    chip->characters = characters;
	for(int i = 0; i < TM1668_REG_NUM; i++)
	{
		chip->reg[0].value = 0;
	}
	return err;
}

int tm1668_close(struct tm1668_chip *chip)
{
	tm1668_set_brightness(chip, 0);
	return 0;
}
