#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>

#define FORMAT_TIME_LENGTH 16

int parse_time(char *time_str);
char *format_time(int minutes);

#endif