#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modulo.h"
#include "entry_list.h"
#include "json.h"

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

Modulo *create_default_modulo(char *username) {
    Modulo *modulo = malloc(sizeof(Modulo));

    // set preferences
    modulo_set_username(modulo, username);
    modulo_set_wakeup_earliest(modulo, DEFAULT_WAKEUP_EARLIEST);
    modulo_set_wakeup_latest(modulo, DEFAULT_WAKEUP_LATEST);
    modulo_set_entry_delimiter(modulo, "%");

    // initialize day pointer
    time_t day_ptr_0 = get_most_recent(DEFAULT_WAKEUP_LATEST);
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

void modulo_set_wakeup_earliest(Modulo *modulo, time_t wakeup) { modulo->wakeup_earliest = wakeup; }
void modulo_set_wakeup_latest(Modulo *modulo, time_t wakeup) { modulo->wakeup_latest = wakeup; }

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

// getters
char *modulo_get_username(Modulo *modulo) { return modulo->username; }
time_t modulo_get_wakeup_earliest(Modulo *modulo) { return modulo->wakeup_earliest; }
time_t modulo_get_wakeup_latest(Modulo *modulo) { return modulo->wakeup_latest; }
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
    time_t now = time(NULL);
    modulo_sync_forward(modulo, 1);
}

void modulo_sync_forward(Modulo *modulo, int days) {

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