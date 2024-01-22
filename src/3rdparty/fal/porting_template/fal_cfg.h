/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-01-19     rgw             first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include "sdk_board.h"

#define FAL_PART_HAS_TABLE_CFG
#define NOR_FLASH_DEV_NAME             "norflash0"

/* ===================== Flash device Configuration ========================= */
extern struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &nor_flash0,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,  "fdb_tsdb1", NOR_FLASH_DEV_NAME,         0, 944*1024, 0}, \
    {FAL_PART_MAGIC_WORD,  "download", NOR_FLASH_DEV_NAME, 944*1024, 1024*1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
