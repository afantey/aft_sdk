/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#include "upm.h"
#include <string.h>

#define DBG_LVL DBG_LOG
#define DBG_TAG "upm.dev"
#include "sdk_log.h"

static const struct upm_stor_dev * const device_table[] = UPM_STOR_DEV_TABLE;
static const size_t device_table_len = sizeof(device_table) / sizeof(device_table[0]);
static uint8_t init_ok = 0;

int upm_stor_dev_init(void)
{
    size_t i;

    if (init_ok)
    {
        return 0;
    }

    for (i = 0; i < device_table_len; i++)
    {
        if(device_table[i]->ops.read == NULL || device_table[i]->ops.write == NULL || device_table[i]->ops.erase == NULL)
        {
            LOG_E("Device Interface read/write/erase invalid\n");
            return -1;
        }
        if (device_table[i]->ops.init)
        {
            device_table[i]->ops.init();
        }
        LOG_D("Storage device | %*.*s | addr: 0x%08lx | len: 0x%08x | blk_size: 0x%08x |initialized finish.\n",
                UPM_NAME_MAX, UPM_NAME_MAX, device_table[i]->name, device_table[i]->addr, device_table[i]->len,
                device_table[i]->blk_size);
    }

    init_ok = 1;
    return 0;
}

/**
 * @brief Finds a storage device by name.
 * @param name The name of the storage device to find.
 * @return A pointer to the storage device struct if found, otherwise NULL.
 */
const struct upm_stor_dev *upm_stor_dev_find(const char *name)
{
    // Check if initialization has been performed successfully
    if(init_ok != 1)
    {
        return NULL;
    }

    size_t i;

    // Loop through device table to find the device with the matching name
    for (i = 0; i < device_table_len; i++)
    {
        if (!strncmp(name, device_table[i]->name, UPM_NAME_MAX)) {
            return device_table[i];
        }
    }

    // Return NULL if device with matching name is not found
    return NULL;
}
