#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "time_utils.h"

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

time_t get_most_recent(int time_minutes) {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    
}

int parse_time(char *time_str) {
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

int parse_12_time(int hour, int minute, char *am_pm) {
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
}

int parse_24_time(int hour, int minute) {
    return hour * 60 + minute;
}

char *format_time(int time_minutes) {
    // hh:mm AM (hh:mm)
    char *formatted = malloc(FORMAT_TIME_LENGTH+1);
    int hours_24 = time_minutes / 60;
    int hours_12 = (12 + hours_24 - 1) % 12 + 1;
    int minutes = time_minutes % 60;
    char *am_pm = hours_24 < 12 ? "AM" : "PM";
    sprintf(formatted, "%02d:%02d %s (%02d:%02d)", hours_12, minutes, am_pm, hours_24, minutes);
    return formatted;
}