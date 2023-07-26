/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __UPM_CFG_H
#define __UPM_CFG_H

#include "sdk_board.h"

extern const struct upm_stor_dev upm_flash_stm32;

/* storage device table */
#define UPM_STOR_DEV_TABLE                                           \
{                                                                    \
    &upm_flash_stm32,                                                 \
}

/* ====================== Partition Configuration ========================== */
// magic_word, partition_name ,stor_dev_name , partition offset on storage, len, reserved
#define UPT_PARTITION_TABLE                                                        \
{                                                                                  \
    {UPM_PART_MAGIC_WORD, "boot"       , "onchip", 0         , 4  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "primary"    , "onchip", 4   * 1024, 60 * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "secondary"  , "onchip", 64  * 1024, 60 * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "bl_param"   , "onchip", 124 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "fdb_kvdb1"  , "onchip", 125 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "fdb_tsdb1"  , "onchip", 126 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "otp"        , "onchip", 127 * 1024, 1  * 1024, 0}, \
}

#endif /* __UPM_CFG_H */
