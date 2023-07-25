#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>

#include "modulo.h"

#define TODAY "today"
#define YESTERDAY "yesterday"
#define TOMORROW "tomorrow"

/* clock time in minutes */
typedef int clk_time_t;

/* offset from day_ptr in seconds */
typedef int offset_t;

#define FORMAT_TIME_LENGTH 16
#define FORMAT_TIME_BUF_SIZE 64

offset_t utc_to_offset(Modulo *modulo, time_t time_utc);
offset_t time_to_offset(Modulo *modulo, clk_time_t time_minutes);

void printf_time(char *format, int time_minutes);
clk_time_t parse_time(char *time_str);

clk_time_t utc_to_time(time_t time_utc);
time_t time_to_utc_next(int time_minutes, time_t ref_point);
time_t time_to_utc_prev(int time_minutes, time_t ref_point);
time_t utc_now();

char *time_to_string(clk_time_t time_minutes);
char *utc_to_string(time_t time_utc, bool use_relative_labels);
char *utc_range_to_string(time_t start_utc, time_t end_utc);

#endif