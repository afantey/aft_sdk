/**
 * Change Logs:
 * Date           Author          Notes
 * 2024-07-25     rgw             first version
 */

#ifndef _SDK_ERROR_H_
#define _SDK_ERROR_H_

#include "sdk_config.h"

#ifndef MODULE_NAME_LENGTH
#define MODULE_NAME_LENGTH 32
#endif

#ifndef FILE_NAME_LENGTH
#define FILE_NAME_LENGTH 32
#endif

#ifndef ERROR_MESSAGE_LENGTH
#define ERROR_MESSAGE_LENGTH 64
#endif

struct error_info {
    uint32_t timestamp;                       // 错误发生的时间戳
    uint32_t line_number;                     // 发生错误的行号
    uint16_t error_code;                      // 错误代码
    uint8_t level;                            // 错误严重程度等级
    char module_name[MODULE_NAME_LENGTH];     // 发生错误的模块名称
    char file_name[FILE_NAME_LENGTH];         // 发生错误的文件名
    char error_message[ERROR_MESSAGE_LENGTH]; // 错误的描述信息
};

void log_error_info(struct error_info *error, uint16_t error_code, uint8_t level,
                    const char *module_name, const char *message,
                    const char *file_name, uint32_t line_number);
void error_info_dump(const struct error_info *error);

#endif /* _SDK_ERROR_H_ */
