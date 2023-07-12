/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-06-25     rgw             first version
 */
#ifndef __SDK_SOFT_TIMER_H
#define __SDK_SOFT_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

struct swtimer
{
    int start;
    int timeout_ms;
    int isstop;
};

int swtimer_expired(struct swtimer *t);
void swtimer_set(struct swtimer *t, int set_timeout_ms);
void swtimer_stop(struct swtimer *t);

#ifdef __cplusplus
}
#endif

#endif /* __SDK_SOFT_TIMER_H */
