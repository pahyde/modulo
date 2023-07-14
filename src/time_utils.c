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

time_t parse_time(char *time_str) {

}