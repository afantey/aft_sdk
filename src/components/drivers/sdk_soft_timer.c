/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-06-25     rgw             first version
 */

#include "sdk_board.h"

static int clock_time(void)
{
    return (int)sdk_hw_get_systick();
}

int swtimer_expired(struct swtimer *t)
{
    if(t->isstop != 1 && t->start != 0)
    {
        return (int)(clock_time() - t->start) >= (int)t->timeout_ms;
    }
    else
    {
        return 0;
    }
}

void swtimer_set(struct swtimer *t, int set_timeout_ms)
{
    t->timeout_ms = set_timeout_ms;
    t->start = clock_time();
    t->isstop = 0;
}

void swtimer_stop(struct swtimer *t)
{
    t->start = 0;
    t->isstop = 1;
}
