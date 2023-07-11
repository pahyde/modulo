#include <cjson/cJSON.h>

#include "json.h"
#include "main.h"

Modulo *json_to_modulo(cJSON *json) {

}

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

cJSON *modulo_to_json(Modulo *modulo) {
    cJSON *json = cJSON_CreateObject(); 

    // add username to JSON
    if (cJSON_AddStringToObject(json, "username", modulo->username) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add wakeup to JSON
    if (cJSON_AddNumberToObject(json, "wakeup", modulo->wakeup) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add today entries to JSON
    if (add_entry_list_to_object(json, "today", modulo->today) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add tomorrow entries to JSON
    if (add_entry_list_to_object(json, "tomorrow", modulo->tomorrow) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add last_update to JSON
    if (cJSON_AddNumberToObject(json, "last_update", modulo->last_update) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }
    return json;
}

cJSON *add_entry_list_to_object(cJSON *json, const char *name, EntryList entry_list) {
    cJSON *array = cJSON_AddArrayToObject(json, name);
    if (array == NULL) {
        return NULL;
    }
    char **entries = entry_list.entries;
    for (int i = 0; i < entry_list.count; i++) {
        cJSON *string = cJSON_CreateString(entries[i]);
        if (string == NULL) {
            // failed to create json string for entry
            return NULL;
        }
        cJSON_AddItemToArray(array, string);
    }
    return array;
}



