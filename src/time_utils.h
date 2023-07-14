#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>

#define FORMAT_TIME_LENGTH 16

char *format_time(time_t utc_time);

#endif