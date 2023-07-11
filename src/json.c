#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "json.h"
#include "main.h"

/*
typedef struct Modulo {
    char *username;
    time_t wakeup;
    EntryList today;
    entry_list tomorrow;
    time_t last_update;
} Modulo;

typedef struct entry_list {
    int count;
    char **entries;
}

json {
    username: string
    wakeup: number (seconds)
    today: [string]
    tomorrow: [string]
    last_update: number (seconds)
}
*/

// json to modulo helpers
static char *get_string_from_object(cJSON *json, char *name);
static time_t get_time_t_from_object(cJSON *json, char *name);
static EntryList get_entry_list_from_object(cJSON *json, char *name);

// modulo to json helpers
static cJSON *add_entry_list_to_object(cJSON *json, const char *name, EntryList entry_list);

Modulo *json_to_modulo(cJSON *json) {
    Modulo *modulo = malloc(sizeof(Modulo));

    char *username = get_string_from_object(json, USERNAME);
    if (username == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    time_t wakeup = get_time_t_from_object(json, WAKEUP);
    if (wakeup == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    EntryList today = get_entry_list_from_object(json, TODAY);
    if (today.count == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    EntryList tomorrow = get_entry_list_from_object(json, TOMORROW);
    if (tomorrow.count == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    time_t last_update = get_time_t_from_object(json, LAST_UPDATE);
    if (last_update == -1) {
        cJSON_Delete(json);
        return NULL;
    }
    modulo->username = username;
    modulo->wakeup = wakeup;
    modulo->today = today;
    modulo->tomorrow = tomorrow;
    modulo->last_update = last_update;

    cJSON_Delete(json);
    return modulo;
}

char *get_string_from_object(cJSON *json, char *name) {
    cJSON *json_string = cJSON_GetObjectItemCaseSensitive(json, name);
    if (json_string == NULL || !cJSON_IsString(json_string) || json_string->valuestring == NULL) {
        // Invalid json string
        return NULL;
    }
    char *c_string = json_string->valuestring;
    char *c_string_cpy = malloc(strlen(c_string) + 1);
    strcpy(c_string_cpy, c_string);
    return c_string_cpy;
}

time_t get_time_t_from_object(cJSON *json, char *name) {
    cJSON *number = cJSON_GetObjectItemCaseSensitive(json, name);
    if (number == NULL || !cJSON_IsNumber(number)) {
        // Invalid json string
        return -1;
    }
    time_t time = (time_t) number->valuedouble;
    return time;
}

/* returns and EntryList with count == -1 on failure */
EntryList get_entry_list_from_object(cJSON *json, char *name) {
    cJSON *array = cJSON_GetObjectItemCaseSensitive(json, name);
    if (array == NULL || !cJSON_IsArray(array)) {
        EntryList err = { -1, NULL };
        return err;
    }

    size_t capacity = 16;
    size_t size = 0;
    char **entries = malloc(capacity * sizeof(char *));
    cJSON *string;

    cJSON_ArrayForEach(string, array) {
        if (!cJSON_IsString(string) || string->valuestring == NULL) {
            // Invalid string array element
            EntryList err = { -1, NULL };
            return err;
        }
        if (size == capacity) {
            capacity *= 2;
            entries = realloc(entries, capacity);
        }
        entries[size++] = string->valuestring;
    }
    EntryList entry_list = { size, entries };
    return entry_list;
}

cJSON *modulo_to_json(Modulo *modulo) {
    cJSON *json = cJSON_CreateObject(); 

    // add username to JSON
    if (cJSON_AddStringToObject(json, USERNAME, modulo->username) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add wakeup to JSON
    if (cJSON_AddNumberToObject(json, WAKEUP, modulo->wakeup) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add today entries to JSON
    if (add_entry_list_to_object(json, TODAY, modulo->today) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add tomorrow entries to JSON
    if (add_entry_list_to_object(json, TOMORROW, modulo->tomorrow) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add last_update to JSON
    if (cJSON_AddNumberToObject(json, LAST_UPDATE, modulo->last_update) == NULL) {
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
