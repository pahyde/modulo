#include "actions.h"

static time_t default_wakeup();

Modulo *create_default_modulo(char *username) {
    Modulo *modulo = malloc(sizeof(Modulo));
    modulo->username = username;
    modulo->wakeup = default_wakeup();
    EntryList today = { 0, NULL };
    modulo->today = today;
    EntryList tomorrow = { 0, NULL };
    modulo->tomorrow = tomorrow;
    time_t now = time(NULL);
    modulo->last_update = now;
    return modulo;
}

void sync(Modulo *modulo) {
    time_t now = time(NULL);
    sync_data_with_timestamp(modulo, now);
}

void sync_data_with_timestamp(Modulo *modulo, time_t now) {
    //TODO
}

time_t default_wakeup() {
    //TODO
    return 0;
}