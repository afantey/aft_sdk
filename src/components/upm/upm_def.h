/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __UPM_DEF_H
#define __UPM_DEF_H

#include <stdint.h>
#include <stdio.h>

#ifndef UPM_NAME_MAX
#define UPM_NAME_MAX 16
#endif

struct upm_stor_dev
{
    char name[UPM_NAME_MAX];

    /* device start address*/
    uint32_t addr;
    /* max len for stroage device */
    size_t len;
    /* block size or page size. e.g. gd32f30x is 1kB */
    size_t blk_size;

    struct
    {
        int (*init)(void);
        int (*read)(long offset, uint8_t *buf, size_t size);
        int (*write)(long offset, const uint8_t *buf, size_t size);
        int (*erase)(long offset, size_t size);
    } ops;
};
typedef struct upm_stor_dev *upm_stor_dev_t;

/**
 * uni partition manager
 */
struct upm_partition
{
    uint32_t magic_word;

    /* partition name */
    char name[UPM_NAME_MAX];
    /* storage device name for partition */
    char stor_dev_name[UPM_NAME_MAX];

    /* partition offset address on storage device */
    long offset;
    size_t len;

    uint32_t reserved;
};
typedef struct upm_partition *upm_partition_t;

#endif /* __UPM_DEF_H */
