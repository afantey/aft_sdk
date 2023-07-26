/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#include "upm.h"
#include <string.h>
#include <stdlib.h>

#define DBG_LVL DBG_LOG
#define DBG_TAG "upm.part"
#include "sdk_log.h"

#define UPM_PART_MAGIC_WORD         0x47575954

#define assert(x)

struct part_stor_info
{
    const struct upm_stor_dev *stor_dev;
};

/* partition table definition */
static const struct upm_partition partition_table_def[] = UPT_PARTITION_TABLE;
static const struct upm_partition *partition_table = NULL;
/* partition object information cache table */
static struct part_stor_info part_stor_cache[sizeof(partition_table_def) / sizeof(partition_table_def[0])] = { 0 };


static uint8_t init_ok = 0;
static size_t partition_table_len = 0;

/**
 * print the partition table
 */
void upm_show_part_table(void)
{
    char *item1 = "name", *item2 = "stor_dev";
    size_t i, part_name_max = strlen(item1), stor_dev_name_max = strlen(item2);
    const struct upm_partition *part;

    if (partition_table_len)
    {
        for (i = 0; i < partition_table_len; i++)
        {
            part = &partition_table[i];
            if (strlen(part->name) > part_name_max)
            {
                part_name_max = strlen(part->name);
            }
            if (strlen(part->stor_dev_name) > stor_dev_name_max)
            {
                stor_dev_name_max = strlen(part->stor_dev_name);
            }
        }
    }
    LOG_I("==================== UPM partition table ====================\n");
    LOG_I("| %-*.*s | %-*.*s |   offset   |    length  |\n", part_name_max, UPM_NAME_MAX, item1, stor_dev_name_max,
            UPM_NAME_MAX, item2);
    LOG_I("-------------------------------------------------------------\n");
    for (i = 0; i < partition_table_len; i++)
    {
        part = &partition_table[i];

        LOG_I("| %-*.*s | %-*.*s | 0x%08lx | 0x%08x |\n", part_name_max, UPM_NAME_MAX, part->name, stor_dev_name_max,
                UPM_NAME_MAX, part->stor_dev_name, part->offset, part->len);
    }
    LOG_I("=============================================================\n");
}

static int check_and_update_part_cache(const struct upm_partition *table, size_t len)
{
    const struct upm_stor_dev *stor_dev = NULL;
    size_t i;

    for (i = 0; i < len; i++)
    {
        stor_dev = upm_stor_dev_find(table[i].stor_dev_name);
        if (stor_dev == NULL)
        {
            LOG_D("Warning: NOT found the storage device(%s).\n", table[i].stor_dev_name);
            continue;
        }

        if (table[i].offset >= (long)stor_dev->len)
        {
            LOG_E("Initialize failed! Partition(%s) offset address(%ld) out of sotrage bound(<%d).\n",
                    table[i].name, table[i].offset, stor_dev->len);
            partition_table_len = 0;

            return -1;
        }

        part_stor_cache[i].stor_dev = stor_dev;
    }

    return 0;
}

int upm_partition_init(void)
{

    if (init_ok)
    {
        return partition_table_len;
    }

    partition_table = &partition_table_def[0];
    partition_table_len = sizeof(partition_table_def) / sizeof(partition_table_def[0]);

    /* check the partition table device exists */
    if (check_and_update_part_cache(partition_table, partition_table_len) != 0)
    {
        goto _exit;
    }

    init_ok = 1;

_exit:

    upm_show_part_table();

    return partition_table_len;
}

const struct upm_partition *upm_partition_find(const char *name)
{
    if(init_ok != 1)
    {
        return NULL;
    }

    size_t i;

    for (i = 0; i < partition_table_len; i++)
    {
        if (!strcmp(name, partition_table[i].name))
        {
            return &partition_table[i];
        }
    }

    return NULL;
}

static const struct upm_stor_dev *stor_dev_find_by_part(const struct upm_partition *part)
{
    assert(part >= partition_table);
    assert(part <= &partition_table[partition_table_len - 1]);

    return part_stor_cache[part - partition_table].stor_dev;
}

const struct upm_partition *upm_get_partition_table(size_t *len)
{
    assert(init_ok);
    assert(len);

    *len = partition_table_len;

    return partition_table;
}


void upm_set_partition_table_temp(struct upm_partition *table, size_t len)
{
    assert(init_ok);
    assert(table);

    check_and_update_part_cache(table, len);

    partition_table_len = len;
    partition_table = table;
}


/**
 * @brief Reads data from a partition.
 * @param part Pointer to the partition to read from.
 * @param addr The relative address to read from within the partition.
 * @param buf Pointer to the buffer to store the read data.
 * @param size The number of bytes to read.
 * @return The number of bytes read, or -1 if an error occurs.
 */
int upm_partition_read(const struct upm_partition *part, uint32_t addr, uint8_t *buf, size_t size)
{
    int ret = 0;
    const struct upm_stor_dev *stor_dev = NULL;

    assert(part);
    assert(buf);

    // Check if partition address is within bounds
    if (addr + size > part->len)
    {
        LOG_E("Partition read error! Partition address out of bound.");
        return -1;
    }

    // Find the storage device associated with the partition
    stor_dev = stor_dev_find_by_part(part);
    if (stor_dev == NULL)
    {
        LOG_E("Partition read error! Don't found storage device(%s) of the partition(%s).", part->stor_dev_name, part->name);
        return -1;
    }

    // Read data from the storage device
    ret = stor_dev->ops.read(part->offset + addr, buf, size);
    if (ret < 0)
    {
        LOG_E("Partition read error! Storage device(%s) read error!", part->stor_dev_name);
    }

    return ret;
}

/**
 * @brief Writes data to a partition.
 * @param part Pointer to the partition to write to.
 * @param addr The relative address to write to within the partition.
 * @param buf Pointer to the data to write.
 * @param size The number of bytes to write.
 * @return The number of bytes written, or -1 if an error occurs.
 */
int upm_partition_write(const struct upm_partition *part, uint32_t addr, const uint8_t *buf, size_t size)
{
    int ret = 0;
    const struct upm_stor_dev *stor_dev = NULL;

    assert(part);
    assert(buf);

    // Check if partition address is within bounds
    if (addr + size > part->len)
    {
        LOG_E("Partition write error! Partition address out of bound.");
        return -1;
    }

    // Find the storage device associated with the partition
    stor_dev = stor_dev_find_by_part(part);
    if (stor_dev == NULL)
    {
        LOG_E("Partition write error! Don't found storage device(%s) of the partition(%s).", part->stor_dev_name, part->name);
        return -1;
    }

    // Write data to the storage device
    ret = stor_dev->ops.write(part->offset + addr, buf, size);
    if (ret < 0)
    {
        LOG_E("Partition write error! Storage device(%s) write error!", part->stor_dev_name);
    }

    return ret;
}

int upm_partition_erase(const struct upm_partition *part, uint32_t addr, size_t size)
{
    int ret = 0;
    const struct upm_stor_dev *stor_dev = NULL;

    assert(part);

    if (addr + size > part->len)
    {
        LOG_E("Partition erase error! Partition address out of bound.");
        return -1;
    }

    stor_dev = stor_dev_find_by_part(part);
    if (stor_dev == NULL)
    {
        LOG_E("Partition erase error! Don't found stroage device(%s) of the partition(%s).", part->stor_dev_name, part->name);
        return -1;
    }

    ret = stor_dev->ops.erase(part->offset + addr, size);
    if (ret < 0)
    {
        LOG_E("Partition erase error! Storage device(%s) erase error!", part->stor_dev_name);
    }

    return ret;
}

int upm_partition_erase_all(const struct upm_partition *part)
{
    return upm_partition_erase(part, 0, part->len);
}
