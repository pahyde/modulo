#include <stdlib.h>
#include <stdio.h>

#include "entry_list.h"

/* EntryList */
EntryList create_entry_list() {
    EntryList entry_list = {
        .send_date = 0,
        .recv_date = 0,
        .read_receipt = false,
        .capacity = ENTRY_LIST_INIT_CAPACITY,
        .size = 0,
        .entries = malloc(ENTRY_LIST_INIT_CAPACITY * sizeof(char *))
    };
    return entry_list;
}

void free_entry_list(EntryList *entry_list) {
    for (int i = 0; i < entry_list->size; i++) {
        free(entry_list_get(entry_list, i));
    }
    free(entry_list->entries);
}

//setters
void entry_list_set_send_date(EntryList *entry_list, time_t send_date) { entry_list->send_date = send_date; }
void entry_list_set_recv_date(EntryList *entry_list, time_t recv_date) { entry_list->recv_date = recv_date; }

void entry_list_set_read_receipt(EntryList *entry_list, bool read_receipt) {
    if (read_receipt != true && read_receipt != false) {
        fprintf(stderr, "read receipt must be either true or false.\n");
    }
    entry_list->read_receipt = read_receipt;
}

// getters
time_t entry_list_get_send_date(EntryList *entry_list) { return entry_list->send_date; }
time_t entry_list_get_recv_date(EntryList *entry_list) { return entry_list->recv_date; }
bool entry_list_get_read_receipt(EntryList *entry_list) { return entry_list->read_receipt; }

// push to entry list and update send_date
void entry_list_push(EntryList *entry_list, char *entry) {
    char **entries = entry_list->entries;
    int *capacity = &entry_list->capacity;
    int *size = &entry_list->size;
    if (*size == *capacity) {
        int new_capacity = *capacity * 2;
        // reallocate entry_list
        entries = realloc(entries, new_capacity * sizeof(char *));
        // update capacity value
        entry_list->capacity = new_capacity;
    }
    // push to entry list
    entries[(*size)++] = entry;
}

char *entry_list_get(EntryList *entry_list, int index) {
    int size = entry_list->size;
    if (index < 0 || index > size-1) {
        fprintf(stderr, "Can't get entry at index %d from EntryList of size %d\n", index, size);
        exit(EXIT_FAILURE);
    }
    return entry_list->entries[index];
}

void entry_list_remove(EntryList *entry_list, int index) {
    int *size = &entry_list->size;
    if (index < 0 || index > *size-1) {
        fprintf(stderr, "Can't remove from EntryList of size %d at index %d\n", *size, index);
        exit(EXIT_FAILURE);
    }
    free(entry_list_get(entry_list, index));
    char **entries = entry_list->entries;
    for (int i = index+1; i < *size; i++) {
        entries[i-1] = entries[i];
    }
    (*size)--;
}

/* HistoryQueue */
HistoryQueue create_history_queue() {
    HistoryQueue history = {
        .head = 0,
        .size = 0
    };
    return history;
}

void free_history_queue(HistoryQueue *history) {
    int size = history->size;
    for (int i = 0; i < size; i++) {
        EntryList *entry_list = history_queue_get(history, i);
        free_entry_list(entry_list);
    }
}

void history_queue_push(HistoryQueue *history, EntryList *entry_list) {
    EntryList *entry_lists = history->entries;
    int *head = &history->head;
    int *size = &history->size;
    int tail_index = (*head + *size) % HISTORY_QUEUE_LENGTH;
    if (*size == 3) {
        // move head to make room
        *head = (*head + 1) % HISTORY_QUEUE_LENGTH;
    } else {
        // increase size
        *size = (*size + 1) % HISTORY_QUEUE_LENGTH;
    }
    entry_lists[tail_index] = *entry_list;
}

EntryList *history_queue_get(HistoryQueue *history, int index) {
    int size = history->size;
    if (index < 0 || index > size-1) {
        fprintf(stderr, "Can't get entry at index %d from HistoryQueue of size %d\n", index, size);
        exit(EXIT_FAILURE);
    }
    int head = history->head;
    return &history->entries[(head + index) % HISTORY_QUEUE_LENGTH];
}