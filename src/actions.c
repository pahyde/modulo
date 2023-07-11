#include "actions.h"
#include "json.h"

static time_t default_wakeup();

//TODO change name to: create_modulo_from_username
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

void free_modulo(Modulo *modulo) {
    free(modulo->username);
    free(modulo->today.entries);
    free(modulo->tomorrow.entries);
    free(modulo);
}

bool out_of_sync(Modulo *modulo) {
    return false;
}

void update_username(Modulo *modulo, char *username) {
    /*
    modulo->username is set by either:
        1. get_system_username -> getenv(c->user_env_variable)
        2. Contents of the argv vector
    
    Both are allocated for the process by the OS,
    so no need to free the old username
    */
    modulo->username = username;
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