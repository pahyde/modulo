#ifndef UTIL_H
#define UTIL_H

#include <time.h>
#include <stdbool.h>

char *path_join(char *path1, char *path2, char separator);
char *format_time(time_t utc_time);

#endif