#ifndef MODULO_H
#define MODULO_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define MODULO_USERNAME "username"
#define MODULO_WAKEUP "wakeup"
#define MODULO_ENTRY_DELIMITER "entry_delimiter"
#define MODULO_TODAY "today"
#define MODULO_TOMORROW "tomorrow"
#define MODULO_LAST_UPDATE "last_update"

#define ENTRY_LIST_CAPACITY 8

typedef struct EntryList {
    int capacity;
    int count;
    char **entries;
} EntryList;

#define USER_NAME_MAX_LEN 31
#define DELIMITER_MAX_LEN 15

typedef struct Modulo {
    char username[USER_NAME_MAX_LEN + 1];
    // wakeup time (minutes)
    int wakeup;
    // the delimiter used to separate journal entries
    char entry_delimiter[DELIMITER_MAX_LEN + 1];
    /*
    EntryList today:
    The entries available for review today.
    These are usually the entries written yesterday
    If the user skips a day, they will be the most 
    recent unread set of entries. 
    */
    EntryList today;
    /* 
    EntryList tomorrow:
    The entries written today that will be available tomorrow 
    */
    EntryList tomorrow;
    /* 
    Today and Tomorrow ptrs (Used for syncing) 
    Points to the utc day number the the respective 
    today and tomorrow entries were (or will be) made available 
    */
    int today_ptr;
    int tomorrow_ptr;
} Modulo;

/*
   Setting wakeup time only effects "tomorrow's" wakeup
   td     tm
   wk     wk
   >>>>>t
   
   set wakeup time an hour earlier
   td   tm
   wk   wk
   >>>>>t

        td   tm
        wk   wk
        >>>>>t
*/


Modulo *create_default_modulo(char *username);
void free_modulo(Modulo *modulo);

// setters
void modulo_set_username(Modulo *modulo, char *username);
void modulo_set_wakeup(Modulo *modulo, time_t wakeup);
void modulo_set_entry_delimiter(Modulo *modulo, char *username);

void modulo_set_today_entries(Modulo *modulo, EntryList entry_list);
void modulo_set_tomorrow_entries(Modulo *modulo, EntryList entry_list);

// getters
char *modulo_get_username(Modulo *modulo);
time_t modulo_get_wakeup(Modulo *modulo);
char *modulo_get_entry_delimiter(Modulo *modulo);

EntryList *modulo_get_today_entries(Modulo *modulo);
EntryList *modulo_get_tomorrow_entries(Modulo *modulo);

// EntryList
EntryList create_entry_list();

// Tomorrow EntryList
void modulo_push_tomorrow(Modulo *modulo, char *entry);
void modulo_remove_tomorrow(Modulo *modulo, int remove_index);

// sync
bool modulo_out_of_sync(Modulo *modulo);
void modulo_sync(Modulo *modulo);
void modulo_sync_with_timestamp(Modulo *modulo, time_t now);

#endif