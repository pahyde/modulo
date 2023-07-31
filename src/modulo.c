#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modulo.h"
#include "entry_list.h"
#include "json.h"
#include "time_utils.h"

/*

Lesson: getters and setters are useful (even for trivial assignment)
We don't know if/when the api for data might change

*/

/*

Lesson: error-values/exceptions vs panic/exit
Prefer communicating up the call stack (over writing to stderr and exiting/panicing)
when the calling function cannot easily predict if the callee will reach an error state

Conversely panic/exit is appropriate when the caller can easily guard against the error
e.g. indexing outside the address boundary of an array, strcpy with strlen(src) > strlen(dest) etc

*/

/*
    modulo_wakeup() {
        // invariant: modulo is synced at this point
        // i.e. day_frame_ptr points to no more than 24 hours ago
        int day_progress = minutes_between(now, modulo->day_frame_ptr);
        if (day_progress >= modulo->wakeup_earliest) {
            modulo_step_days(1)
        } else if (day_progress >= modulo->wakeup_earliest - 2*60*60) {
            "You're up early! Are you sure you want to wakeup and start a new day?"
            yes -> modulo_step_day_frames(1);
        } else {
            // Modulo started the current day at " "
            // Run `modulo set wakeup_earliest` to set an earlier wakeup_time 
            // if you want to start a new day now 
        }
    }

    modulo_sync(time_t now) {
        int days = days_between(now, modulo->day_ptr)
        if (days >= 1) {
            modulo_step_day_frames(days);
        }
    }
*/

static time_t default_wakeup();
static void modulo_increment_day_ptr(Modulo *modulo, int days);

Modulo *create_default_modulo(char *username) {
    Modulo *modulo = malloc(sizeof(Modulo));

    // set preferences
    modulo_set_username(modulo, username);
    modulo_set_wakeup_earliest(modulo, DEFAULT_WAKEUP_EARLIEST);
    modulo_set_wakeup_latest(modulo, DEFAULT_WAKEUP_LATEST);
    modulo_set_entry_delimiter(modulo, "%");

    // initialize day pointer
    time_t day_ptr_0 = time_to_utc_prev(DEFAULT_WAKEUP_LATEST, utc_now());
    modulo_set_day_ptr(modulo, day_ptr_0);

    // initialize today entry lists
    EntryList today = create_entry_list();
    modulo_set_today(modulo, today);

    // initialize tomorrow entry lists
    EntryList tomorrow = create_entry_list();
    modulo_set_tomorrow(modulo, tomorrow);

    // initialize history
    HistoryQueue history = create_history_queue();
    modulo_set_history(modulo, history);
    return modulo;
}

void free_modulo(Modulo *modulo) {
    free_entry_list(&modulo->today);
    free_entry_list(&modulo->tomorrow);
    free_history_queue(&modulo->history);
    free(modulo);
}

void modulo_set_username(Modulo *modulo, char *username) {
    check_length(
        username, 
        USER_NAME_MAX_LEN, 
        "Error: input string too long for username\n"
    );
    strcpy(modulo->username, username);
}

void modulo_set_wakeup_earliest(Modulo *modulo, clk_time_t wakeup) { modulo->wakeup_earliest = wakeup; }
void modulo_set_wakeup_latest(Modulo *modulo, clk_time_t wakeup) { modulo->wakeup_latest = wakeup; }

void modulo_set_entry_delimiter(Modulo *modulo, char *entry_delimiter) {
    check_length(
        entry_delimiter, 
        DELIMITER_MAX_LEN, 
        "Error: input string too long for entry_delimiter\n"
    );
    strcpy(modulo->entry_delimiter, entry_delimiter);
}

void modulo_set_day_ptr(Modulo *modulo, time_t day_ptr) {
    modulo->day_ptr = day_ptr;
}

void modulo_set_today(Modulo *modulo, EntryList entry_list) {
    modulo->today = entry_list;
}
void modulo_set_tomorrow(Modulo *modulo, EntryList entry_list) {
    modulo->tomorrow = entry_list;
}

void modulo_set_history(Modulo *modulo, HistoryQueue history) {
    modulo->history = history;
}

void modulo_push_history(Modulo *modulo, EntryList *entry_list) {
    history_queue_push(&modulo->history, entry_list);
}

// getters
char *modulo_get_username(Modulo *modulo) { return modulo->username; }
clk_time_t modulo_get_wakeup_earliest(Modulo *modulo) { return modulo->wakeup_earliest; }
clk_time_t modulo_get_wakeup_latest(Modulo *modulo) { return modulo->wakeup_latest; }
char *modulo_get_entry_delimiter(Modulo *modulo) { return modulo->entry_delimiter; }

time_t modulo_get_day_ptr(Modulo *modulo) { return modulo->day_ptr; }

EntryList *modulo_get_today(Modulo *modulo) { return &modulo->today; }
EntryList *modulo_get_tomorrow(Modulo *modulo) { return &modulo->tomorrow; }
HistoryQueue *modulo_get_history(Modulo *modulo) { return &modulo->history; }

// Tomorrow EntryList mutation
// TODO: fix
void modulo_push_tomorrow(Modulo *modulo, char *entry) {
    EntryList *tomorrow = modulo_get_tomorrow(modulo);
    entry_list_push(tomorrow, entry);
}

void modulo_remove_tomorrow(Modulo *modulo, int remove_index) {
    EntryList *tomorrow = modulo_get_tomorrow(modulo);
    entry_list_remove(tomorrow, remove_index);
}

// sync
bool modulo_out_of_sync(Modulo *modulo) {
    return false;
}

/*
returns true if sync occurs (day_ptr incremented)
        false otherwise
*/
bool modulo_check_sync(Modulo *modulo) {
    int days_out_of_sync = utc_to_offset(modulo, utc_now()) / (24*60*60);
    if (days_out_of_sync < 1) {
        // wakeup latest hasn't occurred today yet
        return;
    }
    modulo_sync_forward(modulo, days_out_of_sync);
}

void modulo_sync_forward(Modulo *modulo, int days) {
    if (days < 1) {
        return;
    }
    // set tomorrow.recv_date;
    entry_list_set_recv_date(&modulo->tomorrow, utc_now());
    // push today to history if non empty
    if (!entry_list_empty(&modulo->today)) {
        history_queue_push(&modulo->history, &modulo->today); 
    }
    if (days == 1) {
        modulo_set_today(modulo, modulo->tomorrow);
    } else {
        modulo_push_history(modulo, &modulo->tomorrow);
    }
    modulo_set_tomorrow(modulo, create_entry_list());
    modulo_increment_day_ptr(modulo, days);
}

void modulo_increment_day_ptr(Modulo *modulo, int days) {
    // day_ptr -> struct tm -> assert(hour, minute) -> += days -> mktime -> day_ptr
    struct tm *day_ptr_tm = localtime(&modulo->day_ptr);
    clk_time_t day_start = modulo->wakeup_latest;
    int day_start_hour = day_start / 60;
    int day_start_min = day_start % 60;
    if (day_ptr_tm->tm_hour != day_start_hour || day_ptr_tm->tm_min != day_start_min) {
        fprintf(stderr, "day ptr not synced with wakeup_latest\n");
        fprintf(stderr, "day_ptr hour: %d, day_ptr min: %d\n", day_ptr_tm->tm_hour, day_ptr_tm->tm_min);
        day_ptr_tm->tm_hour = day_start_hour;
        day_ptr_tm->tm_min = day_start_min;
    }
    day_ptr_tm->tm_mday += days;
    time_t next_day_ptr = mktime(day_ptr_tm);
    modulo->day_ptr = next_day_ptr;
}

void modulo_sync_with_timestamp(Modulo *modulo, time_t now) {

}

void check_length(char *string, int max_length, char *message) {
    size_t length = strlen(string);
    if (length > max_length) {
        fprintf(stderr, message);
        fprintf(stderr, "max length: %d, input length: %d\n", max_length, length);
        exit(1);
    }
}

time_t default_wakeup() {
    //TODO
    return 0;
}