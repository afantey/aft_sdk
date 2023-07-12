#ifndef __NBOOT_CONFIG_H__
#define __NBOOT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RTT_ENABLE
#define NBOOT_DEBUG

#define IMAGE_META_DATA_ADDR        0x0801F000
#define IMAGE_META_DATA_MAX_SIZE    0x80
#define IMAGE_SLOT_A_START          0x08001000
#define MAX_IMAGE_SIZE              (60*1024)

#ifdef __cplusplus
}
#endif

#endif /* __NBOOT_CONFIG_H__ */
