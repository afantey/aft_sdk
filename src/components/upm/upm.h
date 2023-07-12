/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#ifndef __UPM_H
#define __UPM_H

#include "upm_cfg.h"
#include "upm_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int upm_init(void);

/* =============== storage device operator API =============== */
const struct upm_stor_dev *upm_stor_dev_find(const char *name);

/* =============== partition operator API =============== */
const struct upm *upm_find(const char *name);
const struct upm *upm_get_partition_table(size_t *len);
void upm_set_partition_table_temp(struct upm *table, size_t len);
int upm_partition_read(const struct upm *part, uint32_t addr, uint8_t *buf, size_t size);
int upm_partition_write(const struct upm *part, uint32_t addr, const uint8_t *buf, size_t size);
int upm_partition_erase(const struct upm *part, uint32_t addr, size_t size);
int upm_partition_erase_all(const struct upm *part);
void upm_show_part_table(void);

#ifdef __cplusplus
}
#endif

#endif /* __UPM_H */
