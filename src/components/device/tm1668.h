/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */

#ifndef __TM1668_H
#define __TM1668_H

#include "aft_sdk.h"
#include "sdk_board.h"

#define TM1668_REG_NUM 14

struct tm1668_character {
	uint16_t character;
	uint8_t address;
	uint16_t value;
};

struct tm1668_reg_value{
	uint32_t value;
};

struct tm1668_chip {
	uint32_t gpio_dio, gpio_sclk, gpio_stb;

	int brightness;
	int characters_num;
	struct tm1668_character *characters;
	struct tm1668_reg_value reg[TM1668_REG_NUM];
};

enum tm1668_mode
{
    tm1668_mode_4_digits_13_segments,
    tm1668_mode_5_digits_12_segments,
    tm1668_mode_6_digits_11_segments,
    tm1668_mode_7_digits_10_segments,
};

/* 
 *
 *       0
 *     -----
 *   5|     |1
 *    |  6  |
 *     -----
 *   4|     |2
 *    |     |
 *     -----  .7
 *       3
 * http://en.wikipedia.org/wiki/Seven-segment_display_character_representations
 */

/* Hexadecimal */
#define TM1668_SEG_DIGIT \
	{ 0  , 0x00, 0x03f  /* white 0-*/           }, \
	{ 1  , 0x00, 0x006  /* white 1-*/           }, \
	{ 2  , 0x00, 0x05b  /* white 2-*/           }, \
	{ 3  , 0x00, 0x04f  /* white 3-*/           }, \
	{ 4  , 0x00, 0x066  /* white 4-*/           }, \
	{ 5  , 0x00, 0x06d  /* white 5-*/           }, \
	{ 6  , 0x00, 0x07d  /* white 6-*/           }, \
	{ 7  , 0x00, 0x007  /* white 7-*/           }, \
	{ 8  , 0x00, 0x07f  /* white 8-*/           }, \
	{ 9  , 0x00, 0x06f  /* white 9-*/           }, \
	{ 10 , 0x02, 0x03f  /* white -0*/           }, \
	{ 11 , 0x02, 0x006  /* white -1*/           }, \
	{ 12 , 0x02, 0x05b  /* white -2*/           }, \
	{ 13 , 0x02, 0x04f  /* white -3*/           }, \
	{ 14 , 0x02, 0x066  /* white -4*/           }, \
	{ 15 , 0x02, 0x06d  /* white -5*/           }, \
	{ 16 , 0x02, 0x07d  /* white -6*/           }, \
	{ 17 , 0x02, 0x007  /* white -7*/           }, \
	{ 18 , 0x02, 0x07f  /* white -8*/           }, \
	{ 19 , 0x02, 0x06f  /* white -9*/           }, \
	{ 20 , 0x04, 0x03f  /* red 0-*/             }, \
	{ 21 , 0x04, 0x006  /* red 1-*/             }, \
	{ 22 , 0x04, 0x05b  /* red 2-*/             }, \
	{ 23 , 0x04, 0x04f  /* red 3-*/             }, \
	{ 24 , 0x04, 0x066  /* red 4-*/             }, \
	{ 25 , 0x04, 0x06d  /* red 5-*/             }, \
	{ 26 , 0x04, 0x07d  /* red 6-*/             }, \
	{ 27 , 0x04, 0x007  /* red 7-*/             }, \
	{ 28 , 0x04, 0x07f  /* red 8-*/             }, \
	{ 29 , 0x04, 0x06f  /* red 9-*/             }, \
	{ 30 , 0x06, 0x03f  /* red -0*/             }, \
	{ 31 , 0x06, 0x006  /* red -1*/             }, \
	{ 32 , 0x06, 0x05b  /* red -2*/             }, \
	{ 33 , 0x06, 0x04f  /* red -3*/             }, \
	{ 34 , 0x06, 0x066  /* red -4*/             }, \
	{ 35 , 0x06, 0x06d  /* red -5*/             }, \
	{ 36 , 0x06, 0x07d  /* red -6*/             }, \
	{ 37 , 0x06, 0x007  /* red -7*/             }, \
	{ 38 , 0x06, 0x07f  /* red -8*/             }, \
	{ 39 , 0x06, 0x06f  /* red -9*/             }, \
	{ 40 , 0x08, 0x008  , /* X white */         }, \
	{ 41 , 0x08, 0x004  , /* S */               }, \
	{ 42 , 0x08, 0x002  , /* D */               }, \
	{ 43 , 0x08, 0x001  , /* walk */            }, \
	{ 44 , 0x08, 0x080  , /* BLE blue */        }, \
	{ 45 , 0x08, 0x040  , /* BLE white */       }, \
	{ 46 , 0x08, 0x020  , /* head light */      }, \
	{ 47 , 0x08, 0x010  , /* X blue */          }, \
	{ 48 , 0x0A, 0x008  , /* lock */            }, \
	{ 49 , 0x0A, 0x004  , /* tool */            }, \
	{ 50 , 0x0A, 0x002  , /* temp */            }, \
	{ 51 , 0x0A, 0x001  , /* meter */           }, \
	{ 52 , 0x0A, 0x080  , /* battery 2 red */   }, \
	{ 53 , 0x0A, 0x040  , /* battery 2 white */ }, \
	{ 54 , 0x0A, 0x020  , /* battery 1 red */   }, \
	{ 55 , 0x0A, 0x010  , /* battery 1 white */ }, \
	{ 56 , 0x0C, 0x008  , /* ? */               }, \
	{ 57 , 0x0C, 0x004  , /* battery 5 white */ }, \
	{ 58 , 0x0C, 0x002  , /* battery 4 white */ }, \
	{ 59 , 0x0C, 0x001  , /* battery 3 white */ }, \
	{ 60 , 0x0C, 0x080  , /* ? */               }, \
	{ 61 , 0x0C, 0x040  , /* ? */               }, \
	{ 62 , 0x0C, 0x020  , /* ? */               }, \
	{ 63 , 0x0C, 0x010  , /* ? */               }, \
	{ 64 , 0x02, 0x080  , /* km/h */            }, \
	{ 65 , 0x04, 0x080  , /* left */            }, \
	{ 66 , 0x06, 0x080  , /* right */           }, \
	{ 67 , 0x00, 0x080  , /* mph */             } 

void tm1668_clear(struct tm1668_chip *chip);
void tm1668_set_brightness(struct tm1668_chip *chip, int brightness);
void tm1668_print(struct tm1668_chip *chip, const uint16_t character);
int tm1668_init(struct tm1668_chip *chip);
int tm1668_close(struct tm1668_chip *chip);
void tm1668_print_speed(struct tm1668_chip *chip, uint16_t speed);
void tm1668_print_mode(struct tm1668_chip *chip, uint16_t mode);

#endif //__TM1668_H
