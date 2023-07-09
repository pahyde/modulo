#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <time.h>

typedef struct Modulus {
    char *username;
    uint16_t wakeup;
    char **today_entries;
    char **tomorrow_entries;
    time_t last_update;
} Modulus;


#endif