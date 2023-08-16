
#ifndef __NBOOT_H_
#define __NBOOT_H_

#include <stdint.h>
#include "nboot_config.h"
#include "sdk_board.h"

enum {
    IMAGE_STATE_INVALID = 0,
    IMAGE_STATE_DOWNLOAD_OK,
    IMAGE_STATE_READY_TO_DOWNLOAD,
    IMAGE_STATE_DOWNLOAD_PROCESSING,
};

#define IMAGE_DOWN_MAGIC            0xa5389ace

#if !defined(IMAGE_META_DATA_ADDR)
#define IMAGE_META_DATA_ADDR        0x08008000
#endif

#if !defined(IMAGE_META_DATA_MAX_SIZE)
#define IMAGE_META_DATA_MAX_SIZE    0x800
#endif

#if !defined(IMAGE_SLOT_A_START)
#define IMAGE_SLOT_A_START          0x08010000
#endif

#if !defined(MAX_IMAGE_SIZE)
#define MAX_IMAGE_SIZE              ((64+128)*1024)
#endif

#if !defined(IMAGE_SLOT_A_END)
#define IMAGE_SLOT_A_END            (IMAGE_SLOT_A_START+MAX_IMAGE_SIZE-1)
#endif

#if !defined(IMAGE_SLOT_B_START)
#define IMAGE_SLOT_B_START          (IMAGE_SLOT_A_END+1)
#endif

#if !defined(IMAGE_SLOT_B_END)
#define IMAGE_SLOT_B_END            (IMAGE_SLOT_B_START+MAX_IMAGE_SIZE-1)
#endif

typedef struct {
    uint32_t start;
    uint32_t end;
} Slot_Range;

typedef struct {
    uint32_t magic_bytes;
    uint8_t down_state;
    uint32_t down_size;
} IAP_Images_Down_State;

typedef struct {
    uint32_t flash_start_addr;
    sdk_flash_t *backend;
} Temp_Flash_Context;

/**
 * @brief 获取固件下载存储区，固件下载之前调用（仅适用于片上FLASH）
 * 
 * @return const Slot_Range*    指向固件下载区地址信息
 */
const Slot_Range *nboot_get_down_slot(void);

/**
 * @brief 获取固件下载状态
 * 
 * @param down_state        用于获取固件下载状态的指针
 * @param down_size         用于获取固件大小的指针
 * @return int 
 *                  0       获取成功
 *                  < 0     获取失败
 */
int nboot_get_image_down_state(uint8_t *down_state, uint32_t *down_size);

/**
 * @brief 设置固件下载状态
 * 
 * @param down_state        固件下载状态
 * @param down_size         固件大小
 * @return int 
 *                  0       设置成功
 *                  < 0     设置失败
 */
int nboot_set_image_down_state(uint8_t down_state, uint32_t down_size);

/**
 * @brief 启动应用程序（如果有新固件，先拷贝固件）
 * 
 * @param flash_ctx         临时区（固件下载区）Flash操作上下文
 */
void boot_app(const Temp_Flash_Context *flash_ctx);
void JumpToApp(__IO uint32_t AppAddr);
#endif
