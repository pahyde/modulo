#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>

#define FORMAT_TIME_LENGTH 16

void printf_time(char *format, int time_minutes);

time_t get_next_occurrence(int time_minutes, time_t ref_point);
time_t get_most_recent(int time_minutes, time_t ref_point);

int parse_time(char *time_str);
char *format_time(int minutes);

#endif