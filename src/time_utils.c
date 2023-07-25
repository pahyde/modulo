#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "time_utils.h"
#include "modulo.h"

static int parse_12_time(int hour, int minute, char *am_pm);
static int parse_24_time(int hour, int minute);
static char *tm_to_date_string(struct tm *date, struct tm *ref_date, bool use_relative_labels);
static char *tm_to_time_string(struct tm *time_data);
static struct tm increment_days(struct tm *date, int days);
static bool same_day(struct tm *date1, struct tm *date2);

/*
API
---
representations:
    1) time_of_day int (minutes)
    2) utc_date utc (seconds)
    3) time_specifier input - provided by user input e.g. 9am, 10:15)
    4) time_display output - displayed to user via get wakeup, get_preferences, etc

time_t can communicate either
    1) utc_time_of_day: e.g. 9:00am = (time_t)9 * 60 * 60 
    2) utc_date: e.g. ((year - 1970) * month * day * 24 + 9) * 60 * 60

time_specifier and time_display just communicate time of day


modulo represents wakeup using utc_time_of_day
modulo represents last_update using utc_date
modulo {
    wakeup: int (time of day in minutes)
    today_num: 123442
    tomorrow_num: 123443
}

today = day_number(now, modulo->wakeup)
if (today >= old_tomorrow) {
    // Modulo is out of sync by at least one day
    // today_entries <- tomorrow_entries
    // tomorrow_entries <- empty entry_list
    // today_num <- old_tomorrow
    // tomorrow_num <- new_today + 1
    // This way entries more than one day old can still be read if the user misses a day
} 

conversions
-----------
formatutc_time_of_day -> time_display
parse_time: time_specifier  -> utc_time_of_day

syncing - happens in modulo.c, consumes functions here
syncing is a great example of something that should be atomic
-------
2 ways:
    1) check if last update occurred before most recent wakeup time
    2) check if last update day number is less than current day number
        - i.e. check if day_number(last_update) < day_number(now)
        - where days start at wakeup time
    
    Actually we're probably better off using tm struct from the <time.h> library 
    which (in theory) should be more robust
    
    Modified 1) Use compare logic to check if localtime(last_update) < most_recent_wakeup(localtime(now));
    Modified 2) 

functions
----------
tm most_recent_wakeup(time_t);
int cmp(tm time_1, tm time_2);

time_t parse_time(char *)
*/

/*
    Your input must match one of the following formats:
    AM/PM:
        1. %H(am|pm)
        2. %H:%M(am|pm)
    24-Hour:
        3. %H
        4. %H:%M
    
    Note that white space and leading zeros are optional. Also matching is case insensitive.
    i.e. "9am", "009:00 AM", "9:00am", and "9 : 00" are all valid.
*/

/*
We define offset to be the time in seconds elapsed since the current day pointer

utc_to_offset:  calculated dynamically using the current day pointer
time_to_offset: calculated statically using wakeup_latest in minutes
*/
offset_t utc_to_offset(Modulo *modulo, time_t time_utc) {
    return (int) difftime(time_utc, modulo->day_ptr);
}

offset_t time_to_offset(Modulo *modulo, clk_time_t time_minutes) {
    // minutes in a day
    int modulus = 24 * 60;
    // time of day offset from the day ptr must be positive by definition
    int minutes_offset = ((time_minutes - modulo->wakeup_latest) + modulus) % modulus;
    return minutes_offset * 60;
}

void printf_time(char *format, int time_minutes) {
    char *format_str = time_to_string(time_minutes);
    printf(format, format_str);
    free(format_str);
}

clk_time_t utc_to_time(time_t time_utc) {
    struct tm *local_time = localtime(&time_utc);
    return local_time->tm_hour * 60 + local_time->tm_min;
}

time_t time_to_utc_next(int time_minutes, time_t ref_point) {
    struct tm *local_time = localtime(&ref_point);
    int hour = time_minutes / 60;
    int minute = time_minutes % 60;
    if (local_time->tm_hour > hour) {
        // next occurrence happens tomorrow
        local_time->tm_mday++;
    } else if (local_time->tm_hour == hour && local_time->tm_min > minute) {
        // next occurrence happens tomorrow
        local_time->tm_mday++;
    }
    local_time->tm_hour = hour;
    local_time->tm_min = minute;
    return mktime(local_time);

}

time_t time_to_utc_prev(int time_minutes, time_t ref_point) {
    struct tm *local_time = localtime(&ref_point);
    int hour = time_minutes / 60;
    int minute = time_minutes % 60;
    if (local_time->tm_hour < hour) {
        // most recent occurred yesterday
        local_time->tm_mday--;
    } else if (local_time->tm_hour == hour && local_time->tm_min < minute) {
        // most recent occurred yesterday
        local_time->tm_mday--;
    }
    local_time->tm_hour = hour;
    local_time->tm_min = minute;
    return mktime(local_time);
}

time_t utc_now() {
    return time(NULL);
}

clk_time_t parse_time(char *time_str) {
    int hour = 0;
    int minute = 0;
    char am_pm[3] = {0};
    int matches = 0;
    // scan for hh:mm [am|pm] format
    matches = sscanf(time_str, "%d : %d %2s", &hour, &minute, am_pm);
    if (matches == 3) {
        return parse_12_time(hour, minute, am_pm);
    } else if (matches == 2) {
        return parse_24_time(hour, minute);
    }
    // scan for hh [am|pm] format
    matches = sscanf(time_str, "%d %2s", &hour, am_pm);
    if (matches == 2) {
        return parse_12_time(hour, minute, am_pm);
    } else if (matches == 1) {
        return parse_24_time(hour, minute);
    }
    return -1;
}


char *time_to_string(clk_time_t time_minutes) {
    // hh:mm AM (hh:mm)
    char *formatted = malloc(FORMAT_TIME_LENGTH+1);
    int hours_24 = time_minutes / 60;
    int hours_12 = (12 + hours_24 - 1) % 12 + 1;
    int minutes = time_minutes % 60;
    char *am_pm = hours_24 < 12 ? "AM" : "PM";
    sprintf(formatted, "%02d:%02d %s (%02d:%02d)", hours_12, minutes, am_pm, hours_24, minutes);
    return formatted;
}

char *utc_to_string(time_t time_utc, bool use_relative_labels) {
    static char fmt_string[FORMAT_TIME_BUF_SIZE];

    time_t ref_point_utc = time(NULL);
    struct tm ref_point;
    memcpy(&ref_point, localtime(&ref_point_utc), sizeof(struct tm));
    
    struct tm date;
    memcpy(&date, localtime(&time_utc), sizeof(struct tm));
    sprintf(fmt_string, "%s %s", tm_to_date_string(&date, &ref_point, use_relative_labels), tm_to_time_string(&date));
    return fmt_string;
}

char *utc_range_to_string(time_t start_utc, time_t end_utc) {
    // static wakeup range string
    static char range_fmt_string[FORMAT_TIME_BUF_SIZE];
    /*
    day(start) != day(start) -> individual dates
    day(start) == day(end)   -> converge dates

    date is yesterday -> yesterday
    date is today     -> today
    date is tomorrow  -> tomorrow 
    otherwise         -> date
    */

    // calculate local time now
    time_t now_utc = time(NULL);
    struct tm now;
    memcpy(&now, localtime(&now_utc), sizeof(struct tm));

    // calculate start and end local time
    struct tm start, end;
    memcpy(&start, localtime(&start_utc), sizeof(struct tm));
    memcpy(&end, localtime(&end_utc), sizeof(struct tm));

    // build start format string
    char start_fmt_string[FORMAT_TIME_BUF_SIZE];
    char *start_date = tm_to_date_string(&start, &now, true);
    char *start_time = tm_to_time_string(&start);
    sprintf(start_fmt_string, "%s %s", start_date, start_time);

    // build end format string
    char end_fmt_string[FORMAT_TIME_BUF_SIZE];
    char *end_time = tm_to_time_string(&end);
    if (!same_day(&start, &end)) {
        char *end_date = tm_to_date_string(&end, &now, true);
        sprintf(end_fmt_string, "%s %s", end_date, end_time);
    } else {
        strcpy(end_fmt_string, end_time);
    }
    // combine start and end strings into range format string
    sprintf(range_fmt_string, "%s - %s", start_fmt_string, end_fmt_string);
    return range_fmt_string;
}

bool same_day(struct tm *time1, struct tm *time2) {
    if (time1->tm_year != time2->tm_year) { return false; }
    if (time1->tm_mon  != time2->tm_mon)  { return false; }
    if (time1->tm_mday != time2->tm_mday) { return false; }
    return true;
}


clk_time_t parse_12_time(int hour, int minute, char *am_pm) {
    if (strlen(am_pm) != 2) {
        return -1;
    }
    if (tolower(am_pm[1]) != 'm') {
        return -1;
    }
    if (tolower(am_pm[0]) == 'a') {
        return parse_24_time(hour % 12, minute);
    }
    if (tolower(am_pm[0]) == 'p') {
        return parse_24_time(12 + hour % 12, minute);
    }
    return -1;
}

clk_time_t parse_24_time(int hour, int minute) {
    return hour * 60 + minute;
}

/*
use_relative_labels: whether or not to use date labels like yesterday, today, tomorrow
*/
char *tm_to_date_string(struct tm *date, struct tm *ref_date, bool use_relative_labels) {
    static char fmt_string[FORMAT_TIME_BUF_SIZE];
    if (!use_relative_labels) {
        // return explicit date
        strftime(fmt_string, sizeof fmt_string, "%A, %B %d", date);
        return fmt_string;
    }
    // convert to relative date if applicable (e.g today, yesterday, tomorrow)
    struct tm *today = ref_date;
    struct tm yesterday = increment_days(today, -1);
    struct tm tomorrow = increment_days(today, 1);
    if (same_day(date, today)) {
        strcpy(fmt_string, today);
    } else if (same_day(date, &yesterday)) {
        strcpy(fmt_string, YESTERDAY);
    } else if (same_day(date, &tomorrow)) {
        strcpy(fmt_string, TOMORROW);
    } else {
        strftime(fmt_string, sizeof fmt_string, "%A, %B %d", today);
    }
    return fmt_string;
}

struct tm increment_days(struct tm *date, int days) {
    // normalize incremented date
    date->tm_mday += days;
    time_t date_utc = mktime(date);
    date->tm_mday -= days;
    // convert back to localtime and return
    struct tm *incremented = localtime(&date_utc);
    return *incremented;
}

char *tm_to_time_string(struct tm *date) {
    static char fmt_string[FORMAT_TIME_BUF_SIZE];
    strftime(fmt_string, sizeof fmt_string, "%I:%M %p", date);
    return fmt_string;
}
