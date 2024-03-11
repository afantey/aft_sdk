
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil/sign_key.h"
#include "bootutil_priv.h"
#include "flash_map_backend/image_sign_pub.c.import"
#include "mcuboot_config/mcuboot_logging.h"
#include <stdint.h>

#include "upm.h"

#define BOOTLOADER_START_ADDRESS            0x0
#define BOOTLOADER_SIZE                     (16 * 1024)
#define APPLICATION_PRIMARY_START_ADDRESS   0 // phy start addr is 16 * 1024
#define APPLICATION_SECONDARY_START_ADDRESS 0 // phy start addr is APPLICATION_PRIMARY_START_ADDRESS + APPLICATION_SIZE
#define APPLICATION_SIZE                    (54 * 1024)

#define FLASH_SECTOR_SIZE 1024

static const struct flash_area bootloader = {
    .fa_id = FLASH_AREA_BOOTLOADER,
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = BOOTLOADER_START_ADDRESS,
    .fa_size = BOOTLOADER_SIZE,
};

static const struct flash_area primary_img0 = {
    .fa_id = FLASH_AREA_IMAGE_PRIMARY(0),
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = APPLICATION_PRIMARY_START_ADDRESS,
    .fa_size = APPLICATION_SIZE,
};

static const struct flash_area secondary_img0 = {
    .fa_id = FLASH_AREA_IMAGE_SECONDARY(0),
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = APPLICATION_SECONDARY_START_ADDRESS,
    .fa_size = APPLICATION_SIZE,
};

static const struct flash_area *s_flash_areas[] = {
    &bootloader,
    &primary_img0,
    &secondary_img0,
};

#define ARRAY_SIZE(arr) sizeof(arr) / sizeof(arr[0])

static const struct flash_area *prv_lookup_flash_area(uint8_t id)
{
    for (size_t i = 0; i < ARRAY_SIZE(s_flash_areas); i++)
    {
        const struct flash_area *area = s_flash_areas[i];
        if (id == area->fa_id)
        {
            return area;
        }
    }
    return NULL;
}

int flash_area_open(uint8_t id, const struct flash_area **area_outp)
{
    MCUBOOT_LOG_DBG("%s: ID=%d", __func__, (int)id);

    const struct flash_area *area = prv_lookup_flash_area(id);
    *area_outp = area;

    return area != NULL ? 0 : -1;
}
void flash_area_close(const struct flash_area *fa)
{
    // no need to close
}

//! Reads `len` bytes of flash memory at `off` to the buffer at `dst`
int flash_area_read(const struct flash_area *fa, uint32_t off,
                    void *dst, uint32_t len)
{
    if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH)
    {
        return -1;
    }

    const uint32_t end_offset = off + len;
    if (end_offset > fa->fa_size)
    {
        MCUBOOT_LOG_ERR("%s: Out of Bounds (0x%x vs 0x%x)", __func__, end_offset, fa->fa_size);
        return -1;
    }

    const struct upm *fal_part = NULL;
    if(fa->fa_id == FLASH_AREA_BOOTLOADER)
    {
        fal_part = upm_find("boot");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_PRIMARY(0))
    {
        fal_part = upm_find("primary");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_SECONDARY(0))
    {
        fal_part = upm_find("secondary");
    }
    upm_partition_read(fal_part, off, dst, len);

    return 0;
}
//! Writes `len` bytes of flash memory at `off` from the buffer at `src`
int flash_area_write(const struct flash_area *fa, uint32_t off,
                     const void *src, uint32_t len)
{
    if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH)
    {
        return -1;
    }

    const uint32_t end_offset = off + len;
    if (end_offset > fa->fa_size)
    {
        MCUBOOT_LOG_ERR("%s: Out of Bounds (0x%x vs 0x%x)", __func__, end_offset, fa->fa_size);
        return -1;
    }

    const uint32_t addr = fa->fa_off + off;
    MCUBOOT_LOG_DBG("%s: Addr: 0x%08x Length: %d", __func__, (int)addr, (int)len);

    const struct upm *fal_part = NULL;
    if(fa->fa_id == FLASH_AREA_BOOTLOADER)
    {
        fal_part = upm_find("boot");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_PRIMARY(0))
    {
        fal_part = upm_find("primary");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_SECONDARY(0))
    {
        fal_part = upm_find("secondary");
    }
    upm_partition_write(fal_part, off, src, len);

    return 0;
}

//! Erases `len` bytes of flash memory at `off`
int flash_area_erase(const struct flash_area *fa,
                     uint32_t off, uint32_t len)
{
    if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH)
    {
        return -1;
    }

    if ((len % FLASH_SECTOR_SIZE) != 0 || (off % FLASH_SECTOR_SIZE) != 0)
    {
        MCUBOOT_LOG_ERR("%s: Not aligned on sector Offset: 0x%x Length: 0x%x", __func__,
                        (int)off, (int)len);
        return -1;
    }

    const uint32_t start_addr = fa->fa_off + off;
    MCUBOOT_LOG_DBG("%s: Addr: 0x%08x Length: %d", __func__, (int)start_addr, (int)len);

    const struct upm *fal_part = NULL;
    if(fa->fa_id == FLASH_AREA_BOOTLOADER)
    {
        fal_part = upm_find("boot");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_PRIMARY(0))
    {
        fal_part = upm_find("primary");
    }
    else if(fa->fa_id == FLASH_AREA_IMAGE_SECONDARY(0))
    {
        fal_part = upm_find("secondary");
    }
    upm_partition_erase(fal_part, off, len);

#if VALIDATE_PROGRAM_OP
    for (size_t i = 0; i < len; i++)
    {
        uint8_t *val = (void *)(start_addr + i);
        if (*val != 0xff)
        {
            MCUBOOT_LOG_ERR("%s: Erase at 0x%x Failed", __func__, (int)val);
            assert(0);
        }
    }
#endif

    return 0;
}

//! Returns this `flash_area`s alignment
uint32_t flash_area_align(const struct flash_area *area)
{
    // the smallest unit a flash write can occur along.
    // Note: Image trailers will be scaled by this size
    return 4;
}
//! Returns the value read from an erased flash area byte
uint8_t flash_area_erased_val(const struct flash_area *area)
{
    // the value a byte reads when erased on storage.
    return 0xff;
}

//! Given flash area ID, return info about sectors within the area
int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors)
{
    const struct flash_area *fa = prv_lookup_flash_area(fa_id);
    if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH)
    {
        return -1;
    }

    // All sectors for the NRF52 are the same size
    const size_t sector_size = FLASH_SECTOR_SIZE;
    uint32_t total_count = 0;
    for (size_t off = 0; off < fa->fa_size; off += sector_size)
    {
        // Note: Offset here is relative to flash area, not device
        sectors[total_count].fs_off = off;
        sectors[total_count].fs_size = sector_size;
        total_count++;
    }

    *count = total_count;
    return 0;
}

//! Returns the `fa_id` for slot, where slot is 0 (primary) or 1 (secondary).
//!
//! `image_index` (0 or 1) is the index of the image. Image index is
//!  relevant only when multi-image support support is enabled
int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    switch (slot)
    {
    case 0:
        return FLASH_AREA_IMAGE_PRIMARY(image_index);
    case 1:
        return FLASH_AREA_IMAGE_SECONDARY(image_index);
    }

    MCUBOOT_LOG_ERR("Unexpected Request: image_index=%d, slot=%d", image_index, slot);

    return -1; /* flash_area_open will fail on that */
}
int flash_area_id_from_image_slot(int slot)
{
    return flash_area_id_from_multi_image_slot(0, slot);
}