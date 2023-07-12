/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-11     rgw          first version
 */

#include "upm.h"

#define DBG_LVL DBG_LOG
#define DBG_TAG "upm"
#include "sdk_log.h"

static uint8_t init_ok = 0;

int upm_init(void)
{
    extern int upm_stor_dev_init(void);
    extern int upm_partition_init(void);

    int result;

    result = upm_stor_dev_init();

    if (result < 0) {
        goto __exit;
    }

    result = upm_partition_init();

__exit:

    if ((result > 0) && (!init_ok))
    {
        init_ok = 1;
        LOG_I("Uni Partition Manager initialize success.\n");
    }
    else if(result <= 0)
    {
        init_ok = 0;
        LOG_E("Uni Partition Manager initialize failed.\n");
    }

    return result;
}

int upm_init_check(void)
{
    return init_ok;
}
