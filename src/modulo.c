#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modulo.h"
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

static void free_entry_list(EntryList *entry_list);
static time_t default_wakeup();

Modulo *create_default_modulo(char *username) {
    Modulo *modulo = malloc(sizeof(Modulo));

    // set preferences
    modulo_set_username(modulo, username);
    modulo_set_wakeup(modulo, default_wakeup());
    modulo_set_entry_delimiter(modulo, "%");

    // initialize today entry lists
    EntryList today = create_entry_list();
    modulo_set_today_entries(modulo, today);

    // initialize tomorrow entry lists
    EntryList tomorrow = create_entry_list();
    modulo_set_tomorrow_entries(modulo, tomorrow);

    // time_stamp last update
    modulo_set_last_update(modulo, time(NULL));
    return modulo;
}

void free_modulo(Modulo *modulo) {
    // free(modulo->username);
    free_entry_list(&modulo->today);
    free_entry_list(&modulo->tomorrow);
    free(modulo);
}

void free_entry_list(EntryList *entry_list) {
    char **entries = entry_list->entries;
    for (int i = 0; i < entry_list->count; i++) {
        free(entries[i]);
    }
    free(entries);
}

void modulo_set_username(Modulo *modulo, char *username) {
    check_length(
        username, 
        USER_NAME_MAX_LEN, 
        "Error: input string too long for username\n"
    );
    strcpy(modulo->username, username);
}

void modulo_set_wakeup(Modulo *modulo, time_t wakeup) {
    modulo->wakeup = wakeup;
}

void modulo_set_entry_delimiter(Modulo *modulo, char *entry_delimiter) {
    check_length(
        entry_delimiter, 
        DELIMITER_MAX_LEN, 
        "Error: input string too long for entry_delimiter\n"
    );
    strcpy(modulo->entry_delimiter, entry_delimiter);
}

void modulo_set_today_entries(Modulo *modulo, EntryList entry_list) {
    modulo->today = entry_list;
}
void modulo_set_tomorrow_entries(Modulo *modulo, EntryList entry_list) {
    modulo->tomorrow = entry_list;
}

void modulo_set_last_update(Modulo *modulo, time_t last_update) {
    modulo->last_update = last_update;
}

// getters
char *modulo_get_username(Modulo *modulo) { return modulo->username; }
time_t modulo_get_wakeup(Modulo *modulo) { return modulo->wakeup; }
char *modulo_get_entry_delimiter(Modulo *modulo) { return modulo->entry_delimiter; }

EntryList *modulo_get_today_entries(Modulo *modulo) { return &modulo->today; }
EntryList *modulo_get_tomorrow_entries(Modulo *modulo) { return &modulo->tomorrow; }

time_t modulo_get_last_update(Modulo *modulo, time_t last_update) { return modulo->last_update; }

EntryList create_entry_list() {
    EntryList entry_list = {
        .capacity = 8,
        .count = 0,
        .entries = malloc(ENTRY_LIST_CAPACITY * sizeof(char *))
    };
    return entry_list;
}

// Tomorrow EntryList mutation
void modulo_push_tomorrow(Modulo *modulo, char *entry) {
    EntryList *tomorrow = modulo_get_tomorrow_entries(modulo);
    if (tomorrow->count + 1 > tomorrow->capacity) {
        tomorrow->entries = realloc(tomorrow->entries, tomorrow->capacity * 2);
    }
    tomorrow->entries[tomorrow->count++] = entry;
}

void modulo_remove_tomorrow(Modulo *modulo, int remove_index) {
    EntryList *tomorrow = modulo_get_tomorrow_entries(modulo);
    int count = tomorrow->count;
    if (remove_index >= count) {
        fprintf(stderr, "Error: index %d out of bounds for entry list of length %d\n", remove_index, count);
        exit(1);
    }
    char *removed = tomorrow->entries[remove_index];
    for (int i = remove_index+1; i < count; i++) {
        tomorrow->entries[i-1] = tomorrow->entries[i];
    }
    free(removed);
}

// sync
bool modulo_out_of_sync(Modulo *modulo) {
    return false;
}

void modulo_sync(Modulo *modulo) {
    time_t now = time(NULL);
    modulo_sync_with_timestamp(modulo, now);
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