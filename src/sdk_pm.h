/**
 * Change Logs:
 * Date           Author          Notes
 * 2023-11-21     rgw             first version
 */

#ifndef __SDK_PM_H__
#define __SDK_PM_H__

enum sdk_pm_state
{
    SDK_PM_STATE_NONE,
    SDK_PM_STATE_IDLE,
    SDK_PM_STATE_SLEEP,
    SDK_PM_STATE_STANDBY,
};

struct sdk_pm
{
    enum sdk_pm_state state;
};

int sdk_pm_init(void);
int sdk_pm_deinit(void);

int sdk_pm_suspend(void);
int sdk_pm_resume(void);


#endif /* __SDK_PM_H__ */
