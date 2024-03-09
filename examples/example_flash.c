
#define FLASH_USER_START_ADDR   (FLASH_BASE + FLASH_PAGE_SIZE * 512)             /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE * 10)   /* End @ of user Flash area */

uint8_t data[4] = {1, 2, 3, 4};
uint32_t data32;

uint32_t Address = 0, PAGEError = 0;
Address = FLASH_USER_START_ADDR;
onchip_flash.ops.erase(&onchip_flash, FLASH_USER_START_ADDR, FLASH_PAGE_SIZE * 10);
while (Address < FLASH_USER_END_ADDR)
{
    if (onchip_flash.ops.write(&onchip_flash, Address, data, 4) > 0)
    {
        Address = Address + 4;
    }
    else
    {
        break;
    }
}
Address = FLASH_USER_START_ADDR;
while (Address < FLASH_USER_END_ADDR)
{
    data32 = *(__IO uint32_t *)Address;

    Address = Address + 4;
    LOG_D("%x", data32);
}