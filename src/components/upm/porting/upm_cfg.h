/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __UPM_CFG_H
#define __UPM_CFG_H

#include "sdk_board.h"

extern const struct upm_stor_dev upm_flash_gd32;

/* storage device table */
#define UPM_STOR_DEV_TABLE                                           \
{                                                                    \
    &upm_flash_gd32,                                                 \
}

/* ====================== Partition Configuration ========================== */
// magic_word, partition_name ,stor_dev_name , partition offset on storage, len, reserved
#define UPT_PARTITION_TABLE                                                        \
{                                                                                  \
    {UPM_PART_MAGIC_WORD, "boot"       , "gd32_onchip", 0         , 16 * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "primary"    , "gd32_onchip", 16  * 1024, 54 * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "secondary"  , "gd32_onchip", 70  * 1024, 54 * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "params"     , "gd32_onchip", 124 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "params_bak" , "gd32_onchip", 125 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "bl_param"   , "gd32_onchip", 126 * 1024, 1  * 1024, 0}, \
    {UPM_PART_MAGIC_WORD, "otp"        , "gd32_onchip", 127 * 1024, 1  * 1024, 0}, \
}

#endif /* __UPM_CFG_H */
