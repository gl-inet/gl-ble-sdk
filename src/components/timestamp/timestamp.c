#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "timestamp.h"

uint32_t utils_get_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_usec;
}