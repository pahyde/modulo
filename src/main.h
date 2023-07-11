#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <time.h>

typedef struct Modulo {
    char *username;
    time_t wakeup;
    EntryList today;
    EntryList tomorrow;
    time_t last_update;
} Modulo;

typedef struct EntryList {
    int count;
    char **entries;
} EntryList;

#endif