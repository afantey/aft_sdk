
#include "nboot.h"
#include "sdk_board.h"

void JumpToApp(__IO uint32_t AppAddr)
{
  void (*SysMemBootJump)(void);
    

  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;


  __set_PRIMASK(1);
    

  for (uint32_t i = 0; i < 8; i++)
  {
    NVIC->ICER[i]=0xFFFFFFFF;
    NVIC->ICPR[i]=0xFFFFFFFF;
  } 


  __set_PRIMASK(0);


  SysMemBootJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));


  __set_MSP(*(uint32_t *)AppAddr);


  __set_CONTROL(0);


  SysMemBootJump(); 

  while (1)
  {

  }
}

#ifndef IAP_BUF_SIZE
#define IAP_BUF_SIZE    128
#endif
void boot_app(const Temp_Flash_Context *flash_ctx)
{
    int ret = 0;
    uint8_t iap_buf[IAP_BUF_SIZE];
    uint8_t down_state = 0;
    uint32_t down_size = 0;
    uint32_t img_read_addr = 0;
    uint32_t img_write_addr = IMAGE_SLOT_A_START;
    
    ret = nboot_get_image_down_state(&down_state, &down_size);
    if (ret < 0)
    {
        goto err;
    }
    // Log("down_state %d, down_size %d\r\n", down_state, down_size);
    
    if (down_state == IMAGE_STATE_DOWNLOAD_OK && down_size > 0)
    {
        ret = onchip_flash.ops.erase(&onchip_flash, IMAGE_SLOT_A_START, down_size);
        if (ret < 0)
        {
            goto err;
        }
        img_read_addr = flash_ctx->flash_start_addr;
        for (int i = 0; i < down_size; i += IAP_BUF_SIZE)
        {
            ret = onchip_flash.ops.read(&onchip_flash, img_read_addr, iap_buf, IAP_BUF_SIZE);
            if (ret < 0)
            {
                goto err;
            }
            ret = onchip_flash.ops.write(&onchip_flash, img_write_addr, iap_buf, IAP_BUF_SIZE);
            if (ret < 0)
            {
                goto err;
            }
            img_read_addr += IAP_BUF_SIZE;
            img_write_addr += IAP_BUF_SIZE;
        }
        ret = nboot_set_image_down_state(IMAGE_STATE_INVALID, 0);
        if (ret < 0)
        {
            goto err;
        }
    }

    JumpToApp(IMAGE_SLOT_A_START);

err:
    // Log("boot app failed!\r\n");
    while (1)
    {
    }
}




//slot
///////////////////////////////////////////////////////////////////////////
static uint8_t buffer[IMAGE_META_DATA_MAX_SIZE] = {0};

const Slot_Range down_img_slot =
{
    .start = IMAGE_SLOT_B_START,
    .end = IMAGE_SLOT_B_END
};

static const IAP_Images_Down_State default_state = 
{
    .magic_bytes = IMAGE_DOWN_MAGIC,
    .down_state = IMAGE_STATE_INVALID,
    .down_size = 0,
};

static int load_image_down_state(IAP_Images_Down_State *state)
{
    int ret = 0;

    if (state == NULL)
    {
        return -1;
    }

    ret = onchip_flash.ops.read(&onchip_flash, IMAGE_META_DATA_ADDR, (uint8_t *)state, sizeof(IAP_Images_Down_State));
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

static int save_image_down_state(IAP_Images_Down_State *state)
{
    int ret = 0;

    if (state == NULL)
    {
        return -1;
    }

    ret = onchip_flash.ops.read(&onchip_flash, IMAGE_META_DATA_ADDR, buffer, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    memcpy(buffer, state, sizeof(IAP_Images_Down_State));
    ret = onchip_flash.ops.erase(&onchip_flash, IMAGE_META_DATA_ADDR, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    ret = onchip_flash.ops.write(&onchip_flash, IMAGE_META_DATA_ADDR, buffer, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    return 0;

err:
    return -1;
}

const Slot_Range *nboot_get_down_slot(void)
{
    return &down_img_slot;
}

int nboot_get_image_down_state(uint8_t *down_state, uint32_t *down_size)
{
    int ret = 0;
    IAP_Images_Down_State state = {0};

    ret = load_image_down_state(&state);
    if (ret < 0)
    {
        goto err;
    }

    if (state.magic_bytes == IMAGE_DOWN_MAGIC)
    {
        *down_state = state.down_state;
        *down_size = state.down_size;
    }
    else
    {
        *down_state = default_state.down_state;
        *down_size = default_state.down_size;
    }

    return 0;
err:
    return -1;
}

int nboot_set_image_down_state(uint8_t down_state, uint32_t down_size)
{
    int ret = 0;
    IAP_Images_Down_State state = {0};
    uint8_t need_update = 0;

    ret = load_image_down_state(&state);
    if (ret < 0)
    {
        goto err;
    }
    if (state.magic_bytes == IMAGE_DOWN_MAGIC)
    {
        if (state.down_state != down_state 
            || state.down_size != down_size)
        {
            state.down_state = down_state;
            state.down_size = down_size;
            need_update = 1;
        }
    }
    else
    {
        state.magic_bytes = IMAGE_DOWN_MAGIC;
        state.down_state = down_state;
        state.down_size = down_size;
        need_update = 1;
    }

    if (need_update != 0)
    {
        ret = save_image_down_state(&state);
        if (ret < 0)
        {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

