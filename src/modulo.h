#ifndef MODULO_H
#define MODULO_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "entry_list.h"
#include "time_types.h"


#define MODULO_USERNAME "username"
#define MODULO_WAKEUP_EARLIEST "wakeup_earliest"
#define MODULO_WAKEUP_LATEST "wakeup_latest"
#define MODULO_ENTRY_DELIMITER "entry_delimiter"
#define MOUDLO_DAY_PTR "day_ptr"
#define MODULO_TODAY "today"
#define MODULO_TOMORROW "tomorrow"
#define MODULO_HISTORY "history"

#define DEFAULT_WAKEUP_EARLIEST (6*60)
#define DEFAULT_WAKEUP_LATEST (9*60)

#define USER_NAME_MAX_LEN 31
#define DELIMITER_MAX_LEN 15

/* 
Modulo defines days to start and end at the user specified wakeup_latest time

| day1 start                               day1 end | day2 start
| wakeup_latest -> -> -> -> | wakeup_earliest -> -> | wakeup_latest
^                                                  
day_ptr

- A new day starts automatically after wakeup latest
- Users can alternatively run `modulo wakeup` to start their day early (advance to the next frame)
(modulo guards against `modulo wakeup` being used outside the range wakeup_earliest - wakeup_latest)

2 pieces of information tell us where we are in time
    1. days since day_ptr
    2. minutes since most recent wakeup latest
}
*/ 
typedef struct Modulo {
    char username[USER_NAME_MAX_LEN + 1];
    /* 
    User specified earliest and latest wakeup times
    */ 
    clk_time_t wakeup_earliest;
    clk_time_t wakeup_latest; 
    /* the delimiter typed indicate the end of an entry */
    char entry_delimiter[DELIMITER_MAX_LEN + 1];
    /*
    day_ptr:
    reference utc datetime to the "beginning" of the day 
    corresponds to 24 hours before next wakeup_latest
    Changes to wakeup_latest must be reflected here
    */
    time_t day_ptr;
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
    HistoryQueue - EntryList history
    Contains the 3 most recent EntryLists excluding the today and tomorrow list
    (i.e. the most recent EntryLists written before yesterday)
    */
    HistoryQueue history;
} Modulo;

/*
   Setting wakeup time only effects "tomorrow's" wakeup - slides current dayframe back or forward
*/


Modulo *create_default_modulo(char *username);
void free_modulo(Modulo *modulo);

// setters
void modulo_set_username(Modulo *modulo, char *username);
void modulo_set_wakeup_earliest(Modulo *modulo, clk_time_t wakeup_earliest);
void modulo_set_wakeup_latest(Modulo *modulo, clk_time_t wakeup_latest);
void modulo_set_entry_delimiter(Modulo *modulo, char *username);

void modulo_set_day_ptr(Modulo *modulo, time_t day_ptr);

void modulo_set_today(Modulo *modulo, EntryList entry_list);
void modulo_set_tomorrow(Modulo *modulo, EntryList entry_list);
void modulo_set_history(Modulo *modulo, HistoryQueue history);

// getters
char *modulo_get_username(Modulo *modulo);
clk_time_t modulo_get_wakeup_earliest(Modulo *modulo);
clk_time_t modulo_get_wakeup_latest(Modulo *modulo);
char *modulo_get_entry_delimiter(Modulo *modulo);

time_t modulo_get_day_ptr(Modulo *modulo);

EntryList *modulo_get_today(Modulo *modulo);
EntryList *modulo_get_tomorrow(Modulo *modulo);
HistoryQueue *modulo_get_history(Modulo *modulo);

// EntryList
void modulo_push_tomorrow(Modulo *modulo, char *entry);
void modulo_remove_tomorrow(Modulo *modulo, int remove_index);

// sync
bool modulo_check_sync(Modulo *modulo);
void modulo_sync_forward(Modulo *modulo, int days);
void modulo_sync_with_timestamp(Modulo *modulo, time_t now);

#endif