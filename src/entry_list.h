#ifndef ENTRY_LIST_H
#define ENTRY_LIST_H

/*
Lesson: again don't cater the api to one use case. Let that use case write it's own convenience functions
*/

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define ENTRY_LIST_SEND_DATE "send_date"
#define ENTRY_LIST_RECV_DATE "recv_date"
#define ENTRY_LIST_READ_RECEIPT "read_receipt"
#define ENTRY_LIST_ENTRIES "entries"

#define ENTRY_LIST_INIT_CAPACITY 8

typedef struct EntryList {
    /* The last datetime in which the EntryList was modified */
    time_t send_date;
    /* The datetime in which the EntryList was transferred to the next days "today list" */
    time_t recv_date;
    bool read_receipt;
    int capacity;
    int size;
    char **entries;
} EntryList;

#define HISTORY_QUEUE_LENGTH 3

typedef struct HistoryQueue {
    uint8_t head;
    uint8_t size;
    EntryList entries[HISTORY_QUEUE_LENGTH];
} HistoryQueue;

/* EntryList */
EntryList create_entry_list();
void free_entry_list(EntryList *entry_list);

//setters
void entry_list_set_send_date(EntryList *entry_list, time_t send_date);
void entry_list_set_recv_date(EntryList *entry_list, time_t send_date);
void entry_list_set_read_receipt(EntryList *entry_list, bool read_receipt);

// getters
void entry_list_get_send_date(EntryList *entry_list);
void entry_list_get_recv_date(EntryList *entry_list);
void entry_list_get_read_receipt(EntryList *entry_list);

// push to entry list
void entry_list_push(EntryList *entry_list, char *entry);
// get entry at index
char *entry_list_get(EntryList *entry_list, int index);
// remove entry at index
void entry_list_remove(EntryList *entry_list, int index);

/* HistoryQueue */
HistoryQueue create_history_queue();
void free_history_queue(HistoryQueue *history);

void history_queue_push(HistoryQueue *history, EntryList *entry_list);
EntryList *history_queue_get(HistoryQueue *history, int index);

#endif