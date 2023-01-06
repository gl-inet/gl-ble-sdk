/*****************************************************************************
 Copyright 2022 GL-iNet. https://www.gl-inet.com/

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

#include "gl_log.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#ifdef SYSLOG
#include <syslog.h>
#endif

/* 31, red. 32, green. 33, yellow. 34, blue. 35, magenta. 36, cyan. 37, white. */
// char* lvl_color[] = {
//     "[0m", "[1;31m", "[1;33m", "[1;32m", "[1;34m"
// };
log_name_t lvl_names[] = {
    { "emerg", LOG_EMERG_LEVEL },
    { "alert", LOG_ALERT_LEVEL },
    { "crit", LOG_CRIT_LEVEL },
    { "err", LOG_ERROR_LEVEL },
    { "warning", LOG_WARN_LEVEL },
    { "notice", LOG_NOTICE_LEVEL },
    { "info", LOG_INFO_LEVEL },
    { "debug", LOG_DEBUG_LEVEL },
    { NULL, -1 }
};

log_color_t lvl_color[] = {
    { "[0m", LOG_EMERG_LEVEL },
    { "[0m", LOG_ALERT_LEVEL },
    { "[0m", LOG_CRIT_LEVEL },
    { "[1;31m", LOG_ERROR_LEVEL },
    { "[1;33m", LOG_WARN_LEVEL },
    { "[0m", LOG_NOTICE_LEVEL },
    { "[1;32m", LOG_INFO_LEVEL },
    { "[1;34m", LOG_DEBUG_LEVEL },
    { NULL, -1 }
};

#if defined(ENABLE_DEBUG)
log_level_t g_log_level = LOG_DEBUG_LEVEL;
#else
log_level_t g_log_level = LOG_INFO_LEVEL;
#endif

static const char* _get_filename(const char* p)
{
#ifdef WIN32
    char ch = '\\';
#else
    char ch = '/';
#endif
    const char* q = strrchr(p, ch);
    if (q == NULL) {
        q = p;
    } else {
        q++;
    }
    return q;
}

void IOT_SetLogLevel(log_level_t level)
{
    g_log_level = level;
}

log_level_t IOT_GetLogLevel(void)
{
    return g_log_level;
}

void IOT_LogPrintf(const char* module, const char* file, const char* func, const int line, const int level, const char* fmt, ...)
{
    if (level > g_log_level) {
        return;
    }

    /* format log content */
    const char* file_name = _get_filename(file);

    char sg_text_buf[MAX_LOG_MSG_LEN + 1];
    char* tmp_buf = sg_text_buf;
    char* o = tmp_buf;
    memset(tmp_buf, 0, sizeof(sg_text_buf));

    if (NULL != module) {
        o += snprintf(o, sizeof(sg_text_buf), "%s|%s|%s|%s(%d): ", lvl_names[level].c_name, module, file_name, func, line);
    } else {
        o += snprintf(o, sizeof(sg_text_buf), "%s|%s|%s(%d): ", lvl_names[level].c_name, file_name, func, line);
    }

    va_list ap;
    va_start(ap, fmt);
    o += vsnprintf(o, MAX_LOG_MSG_LEN - 2 - strlen(tmp_buf), fmt, ap);
    va_end(ap);

    strcat(tmp_buf, "\r\n");
#ifndef SYSLOG
    printf("%s%s", "\033", lvl_color[level].c_color);
#endif

    if (level <= g_log_level) {
#ifdef SYSLOG
        syslog(level, "%s", tmp_buf);
#else
        /* default log handler: print to console */
        printf("%s", tmp_buf);
#endif
    }

#ifndef SYSLOG
    printf("%s", "\033[0m");
#endif

    return;
}

#define LITE_HEXDUMP_DRAWLINE(start_mark, len, end_mark) \
    do {                                                 \
        int i;                                           \
                                                         \
        printf("%s", start_mark);                        \
        for (i = 0; i < len; ++i) {                      \
            printf("-");                                 \
        }                                                \
        printf("%s", end_mark);                          \
        printf("\r\n");                                  \
                                                         \
    } while (0)

#define HEXDUMP_SEP_LINE "+"                       \
                         "-----------------------" \
                         "-----------------------" \
                         "-----------------------"
void utils_log_hexdump(const char* title, const void* buff, const int len)
{
    int i, j, written;
    unsigned char ascii[20] = { 0 };
    char header[64] = { 0 };
    unsigned char* buf = (unsigned char*)buff;

    snprintf(header, sizeof(header), "| %s: (len=%d) |\r\n", title, (int)len);

    LITE_HEXDUMP_DRAWLINE("+", strlen(header) - 4, "+");
    printf("%s", header);
    printf("%s\r\n", HEXDUMP_SEP_LINE);

    written = 0;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0) {
            printf("| %08X: ", (unsigned int)(i + (long)buff));
            written += 8;
        }

        printf("%02X", buf[i]);
        written += 2;

        if (i % 2 == 1) {
            printf(" ");
            written += 1;
        }
        snprintf((char*)ascii + i % 16, (1 + 1), "%c", ((buf[i] >= ' ' && buf[i] <= '~') ? buf[i] : '.'));

        if (((i + 1) % 16 == 0) || (i == len - 1)) {
            for (j = 0; j < 48 - written; ++j) {
                printf(" ");
            }

            printf(" %s", ascii);
            printf("\r\n");

            written = 0;
            memset(ascii, 0, sizeof(ascii));
        }
    }
    printf("%s\r\n", HEXDUMP_SEP_LINE);

    return ;
}
