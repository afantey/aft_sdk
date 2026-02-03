/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-07-25     rgw             first version
 */
#include "aft_sdk.h"
#include "sdk_error.h"

#define DBG_TAG "err"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

/*
void my_function()
{
    // ...

    if (error_condition)
    {
        struct error_info error;
        log_error_info(&error, ERROR_CODE_MY_ERROR, DBG_ERROR, "MyModule", 
                        "Something went wrong!", __FILE__, __LINE__);

        // ...
    }

    // ...
}
*/

void log_error_info(struct error_info *error, uint16_t error_code, uint8_t level,
                    const char *module_name, const char *message,
                    const char *file_name, uint32_t line_number)
{
    // 初始化 error_info 结构体
    error->timestamp   = rt_tick_get(); // 获取当前系统tick
    error->error_code  = error_code;
    error->level       = level;

    memset(error->module_name, 0, sizeof(error->module_name));
    memset(error->file_name, 0, sizeof(error->file_name));
    memset(error->error_message, 0, sizeof(error->error_message));

    strncpy(error->module_name, module_name, sizeof(error->module_name) - 1);
    strncpy(error->file_name, file_name, sizeof(error->file_name) - 1);
    error->line_number = line_number;
    strncpy(error->error_message, message, sizeof(error->error_message) - 1);
}

void error_info_dump(const struct error_info *error)
{
    // 定义日志级别字符串数组，与 level 值对应
    const char *level_str[] = {
        "NONE", "ERROR", "WARNING", "INFO", "LOG"};

    uint8_t level = error->level;

    LOG_RAW("Error Info:");
    LOG_RAW("  Timestamp: %lu", error->timestamp);
    LOG_RAW("  Error Code: %u", error->error_code);
    LOG_RAW("  Level: %s", level_str[level]);
    LOG_RAW("  Module: %s", error->module_name);
    LOG_RAW("  File: %s", error->file_name);
    LOG_RAW("  Line: %lu", error->line_number);
    LOG_RAW("  Message: %s", error->error_message);
}
