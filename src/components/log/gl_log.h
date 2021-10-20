/*****************************************************************************
 Copyright 2020 GL-iNet. https://www.gl-inet.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/

#ifndef _GL_LOG_H_
#define _GL_LOG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>

#define MAX_LOG_MSG_LEN 512

#define LOG_DIS "DIS"
#define LOG_ERR "ERR"
#define LOG_WRN "WRN"
#define LOG_INF "INF"
#define LOG_DBG "DBG"

typedef enum {
    LOG_EMERG_LEVEL = 0,
    LOG_ALERT_LEVEL = 1,
    LOG_CRIT_LEVEL = 2,
    LOG_ERROR_LEVEL = 3,
    LOG_WARN_LEVEL = 4,
    LOG_NOTICE_LEVEL = 5,
    LOG_INFO_LEVEL = 6,
    LOG_DEBUG_LEVEL = 7
} log_level_t;

typedef struct {
    char* c_name;
    int c_val;
} log_name_t;

typedef struct {
    char* c_color;
    int c_val;
} log_color_t;

void IOT_SetLogLevel(log_level_t level);
log_level_t IOT_GetLogLevel(void);

void IOT_LogPrintf(const char* module, const char* file, const char* func, const int line, const int level, const char* fmt, ...);
void utils_log_hexdump(const char* title, const void* buff, const int len);

// 全局日志输出宏
#if 1
#define log_debug(fmt, ...) IOT_LogPrintf(NULL, __FILE__, __FUNCTION__, __LINE__, LOG_DEBUG_LEVEL, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) IOT_LogPrintf(NULL, __FILE__, __FUNCTION__, __LINE__, LOG_INFO_LEVEL, fmt, ##__VA_ARGS__)
#define log_warning(fmt, ...) IOT_LogPrintf(NULL, __FILE__, __FUNCTION__, __LINE__, LOG_WARN_LEVEL, fmt, ##__VA_ARGS__)
#define log_err(fmt, ...) IOT_LogPrintf(NULL, __FILE__, __FUNCTION__, __LINE__, LOG_ERROR_LEVEL, fmt, ##__VA_ARGS__)

// #define log_debug(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n");
// #define log_info(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n");
// #define log_warning(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n");
// #define log_err(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n");

#else
#define log_debug(fmt, ...) \
    do {                    \
        ;                   \
    } while (0)
#define log_info(fmt, ...) \
    do {                   \
        ;                  \
    } while (0)
#define log_warning(fmt, ...) \
    do {                      \
        ;                     \
    } while (0)
#define log_err(fmt, ...) \
    do {                  \
        ;                 \
    } while (0)
#endif

// 基于模块级别的日志输出宏
#if 1
#define mlog_debug(module, fmt, ...) IOT_LogPrintf(module, __FILE__, __FUNCTION__, __LINE__, LOG_DEBUG_LEVEL, fmt, ##__VA_ARGS__)
#define mlog_info(module, fmt, ...) IOT_LogPrintf(module, __FILE__, __FUNCTION__, __LINE__, LOG_INFO_LEVEL, fmt, ##__VA_ARGS__)
#define mlog_warning(module, fmt, ...) IOT_LogPrintf(module, __FILE__, __FUNCTION__, __LINE__, LOG_WARN_LEVEL, fmt, ##__VA_ARGS__)
#define mlog_err(module, fmt, ...) IOT_LogPrintf(module, __FILE__, __FUNCTION__, __LINE__, LOG_ERROR_LEVEL, fmt, ##__VA_ARGS__)
#else
#define mlog_debug(fmt, ...) \
    do {                     \
        ;                    \
    } while (0)
#define mlog_info(fmt, ...) \
    do {                    \
        ;                   \
    } while (0)
#define mlog_warning(fmt, ...) \
    do {                       \
        ;                      \
    } while (0)
#define mlog_err(fmt, ...) \
    do {                   \
        ;                  \
    } while (0)
#endif

// 16进制打印
#define log_hexdump(buf, len) utils_log_hexdump(#buf, (const void*)buf, (const int)len)

#if defined(__cplusplus)
}
#endif

#endif /* _INFRA_LOG_H_ */
