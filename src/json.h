#ifndef JSON_H
#define JSON_H

#include <cjson/cJSON.h>

#include "main.h"

#define USERNAME "username"
#define WAKEUP "wakeup"
#define TODAY "today"
#define TOMORROW "tomorrow"
#define LAST_UPDATE "last_update"

/*
typedef struct Modulo {
    char *username;
    time_t wakeup;
    entry_list today;
    entry_list tomorrow;
    time_t last_update;
} Modulo;

typedef struct entry_list {
    int count;
    char **entries;
}

JSON {
    username: string
    wakeup: number (seconds)
    today: [string]
    tomorrow: [string]
    last_update: number (seconds)
}
*/

Modulo *json_to_modulo(cJSON *json);
cJSON *modulo_to_json(Modulo *modulo);

#endif