#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>

#define FORMAT_TIME_LENGTH 16

void printf_time(char *format, int time_minutes);

int datetime_offset(time_t ref_point, Modulo *modulo);
int time_of_day_offset(int time_minutes, Modulo *modulo);

int time_of_day(time_t ref_point);
time_t get_next_occurrence(int time_minutes, time_t ref_point);
time_t get_most_recent(int time_minutes, time_t ref_point);

int parse_time(char *time_str);
char *format_time(int minutes);
char *format_wakeup_range(Modulo *modulo);

#endif