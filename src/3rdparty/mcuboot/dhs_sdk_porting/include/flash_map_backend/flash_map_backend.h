/**
 * Change Logs:
 * Date           Author       Notes
 * {data}         rgw          first version
 */
#ifndef __FLASH_MAP_BACKEND_H__
#define __FLASH_MAP_BACKEND_H__

#include <stdint.h>
#include "mcuboot_config/mcuboot_config.h"

struct flash_area {
    uint8_t  fa_id;         /** The slot/scratch identification */
    uint8_t  fa_device_id;  /** The device id (usually there's only one) */
    uint16_t pad16;
    uint32_t fa_off;        /** The flash offset from the beginning */
    uint32_t fa_size;       /** The size of this sector */
};


/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: flash_area_get_id
 *
 * Description:
 *   Obtain the ID of a given flash area.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   The ID of the requested flash area.
 *
 ****************************************************************************/

static inline uint8_t flash_area_get_id(const struct flash_area *fa)
{
  return fa->fa_id;
}

/****************************************************************************
 * Name: flash_area_get_device_id
 *
 * Description:
 *   Obtain the ID of the device in which a given flash area resides on.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   The device ID of the requested flash area.
 *
 ****************************************************************************/

static inline uint8_t flash_area_get_device_id(const struct flash_area *fa)
{
  return fa->fa_device_id;
}

/****************************************************************************
 * Name: flash_area_get_off
 *
 * Description:
 *   Obtain the offset, from the beginning of a device, where a given flash
 *   area starts at.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   The offset value of the requested flash area.
 *
 ****************************************************************************/

static inline uint32_t flash_area_get_off(const struct flash_area *fa)
{
  return fa->fa_off;
}

/****************************************************************************
 * Name: flash_area_get_size
 *
 * Description:
 *   Obtain the size, from the offset, of a given flash area.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   The size value of the requested flash area.
 *
 ****************************************************************************/

static inline uint32_t flash_area_get_size(const struct flash_area *fa)
{
  return fa->fa_size;
}

/**
 * @brief Structure describing a sector within a flash area.
 *
 * Each sector has an offset relative to the start of its flash area
 * (NOT relative to the start of its flash device), and a size. A
 * flash area may contain sectors with different sizes.
 */
struct flash_sector {
    /**
     * Offset of this sector, from the start of its flash area (not device).
     */
    uint32_t fs_off;

    /**
     * Size of this sector, in bytes.
     */
    uint32_t fs_size;
};
/****************************************************************************
 * Name: flash_sector_get_off
 *
 * Description:
 *   Obtain the offset, from the beginning of its flash area, where a given
 *   flash sector starts at.
 *
 * Input Parameters:
 *   fs - Flash sector.
 *
 * Returned Value:
 *   The offset value of the requested flash sector.
 *
 ****************************************************************************/

static inline uint32_t flash_sector_get_off(const struct flash_sector *fs)
{
    return fs->fs_off;
}

/****************************************************************************
 * Name: flash_sector_get_size
 *
 * Description:
 *   Obtain the size, from the offset, of a given flash sector.
 *
 * Input Parameters:
 *   fs - Flash sector.
 *
 * Returned Value:
 *   The size in bytes of the requested flash sector.
 *
 ****************************************************************************/

static inline uint32_t flash_sector_get_size(const struct flash_sector *fs)
{
    return fs->fs_size;
}

//! Opens the area for use. id is one of the `fa_id`s */
int flash_area_open(uint8_t id, const struct flash_area **area_outp);
void flash_area_close(const struct flash_area *fa);

//! Reads `len` bytes of flash memory at `off` to the buffer at `dst`
int flash_area_read(const struct flash_area *fa, uint32_t off,
                    void *dst, uint32_t len);
//! Writes `len` bytes of flash memory at `off` from the buffer at `src`
int flash_area_write(const struct flash_area *fa, uint32_t off,
                     const void *src, uint32_t len);
//! Erases `len` bytes of flash memory at `off`
int flash_area_erase(const struct flash_area *fa,
                     uint32_t off, uint32_t len);

//! Returns this `flash_area`s alignment
uint32_t flash_area_align(const struct flash_area *area);
//! Returns the value read from an erased flash area byte
uint8_t flash_area_erased_val(const struct flash_area *area);

//! Given flash area ID, return info about sectors within the area
int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors);

//! Returns the `fa_id` for slot, where slot is 0 (primary) or 1 (secondary).
//!
//! `image_index` (0 or 1) is the index of the image. Image index is
//!  relevant only when multi-image support support is enabled
int flash_area_id_from_multi_image_slot(int image_index, int slot);
int flash_area_id_from_image_slot(int slot);

#endif
