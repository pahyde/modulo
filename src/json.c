#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "json.h"
#include "modulo.h"


/*
Lesson: Iterators (like getters/setters) are another example 
of creating separation between the desired outcome of a computation
and the implementation. This gives us flexibility and control when
requirements change
*/


/*
typedef struct EntryList {
    time_t send_date;
    time_t recv_date;
    bool read_receipt;
    int capacity;
    int count;
    char **entries;
} EntryList;

typedef struct EntryListHistory {
    uint8_t head;
    uint8_t size;
    EntryList queue[3];
} EntryListHistory;

 
typedef struct Modulo {
    char username[USER_NAME_MAX_LEN + 1];
    int wakeup_earliest;
    int wakeup_latest; 
    char entry_delimiter[DELIMITER_MAX_LEN + 1];
    time_t day_ptr;
    EntryList today;
    EntryList tomorrow;
    EntryListHistory history;
} Modulo;

typedef struct EntryList {
    time_t send_date;
    time_t recv_date;
    bool read_receipt;
    int capacity;
    int count;
    char **entries;
} EntryList;

typedef struct EntryListHistory {
    uint8_t head;
    uint8_t size;
    EntryList queue[3];
} EntryListHistory;

json {
    username: string
    wakeup_earliest: number (minutes)
    wakeup_latest: number (minutes)
    entry_delimiter: string
    day_ptr: number (seconds)
    today: {
        send_date: number
        recv_date: number
        read_receipt: bool
        entries: []
    }
    tomorrow: {
        send_date: number
        recv_date: number
        read_receipt: bool
        entries: []
    }
    history: [
        {
            send_date: number
            recv_date: number
            read_receipt: bool
            entries: []
        }
    ]
}


*/



// json to modulo helpers
static EntryList json_to_entry_list(cJSON *json);
static HistoryQueue json_to_history_queue(cJSON *json);
static char *get_string_from_object(cJSON *json, char *name);
static int get_int_from_object(cJSON *json, char *name);
static time_t get_time_t_from_object(cJSON *json, char *name);
static EntryList get_entry_list_from_object(cJSON *json, char *name);
static HistoryQueue get_history_queue_from_object(cJSON *json);

// modulo to json helpers
static cJSON *entry_list_to_json(EntryList *entry_list);
static cJSON *history_queue_to_json(HistoryQueue *history);
static cJSON *add_entry_list_to_object(cJSON *json, const char *name, EntryList *entry_list);
static cJSON *add_history_queue_to_object(cJSON *json, const char *name, HistoryQueue *history);

Modulo *json_to_modulo(cJSON *json) {
    Modulo *modulo = malloc(sizeof(Modulo));

    char *username = get_string_from_object(json, MODULO_USERNAME);
    if (username == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    int wakeup_earliest = get_int_from_object(json, MODULO_WAKEUP_EARLIEST);
    if (wakeup_earliest == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    int wakeup_latest = get_int_from_object(json, MODULO_WAKEUP_LATEST);
    if (wakeup_latest == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    char *entry_delimiter = get_string_from_object(json, MODULO_ENTRY_DELIMITER);
    if (username == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    time_t day_ptr = get_time_t_from_object(json, MOUDLO_DAY_PTR);
    if (day_ptr == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    EntryList today = get_entry_list_from_object(json, MODULO_TODAY);
    if (today.size == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    EntryList tomorrow = get_entry_list_from_object(json, MODULO_TOMORROW);
    if (tomorrow.size == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    HistoryQueue history = get_history_queue_from_object(json);
    if (tomorrow.size == -1) {
        cJSON_Delete(json);
        return NULL;
    }

    modulo_set_username(modulo, username);
    modulo_set_wakeup_earliest(modulo, wakeup_earliest);
    modulo_set_wakeup_latest(modulo, wakeup_latest);
    modulo_set_entry_delimiter(modulo, entry_delimiter);
    modulo_set_day_ptr(modulo, day_ptr);
    modulo_set_today(modulo, today);
    modulo_set_tomorrow(modulo, tomorrow);
    modulo_set_history(modulo, history);
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
    return c_string;
}

int get_int_from_object(cJSON *json, char *name) {
    cJSON *number = cJSON_GetObjectItemCaseSensitive(json, name);
    if (number == NULL || !cJSON_IsNumber(number)) {
        // Invalid json number
        return -1;
    }
    int integer = (int) number->valuedouble;
    return integer;
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

EntryList get_entry_list_from_object(cJSON *json, char *name) {
    cJSON *json_entry_list = cJSON_GetObjectItemCaseSensitive(json, name);
    return json_to_entry_list(json_entry_list);
}

HistoryQueue get_history_queue_from_object(cJSON *json) {
    cJSON *json_history_queue = cJSON_GetObjectItemCaseSensitive(json, MODULO_HISTORY);
    return json_to_history_queue(json_history_queue);
}

/*

typedef struct EntryList {
    time_t send_date;
    time_t recv_date;
    bool read_receipt;
    int capacity;
    int count;
    char **entries;
} EntryList;

*/
EntryList json_to_entry_list(cJSON *json) {
    if (json == NULL || !cJSON_IsObject(json)) {
        return (EntryList) { .size = -1 };
    }
    // get send date
    time_t send_date = get_time_t_from_object(json, ENTRY_LIST_SEND_DATE);
    if (send_date == -1) {
        return (EntryList) { .size = -1 };
    }
    // get recv_date
    time_t recv_date = get_time_t_from_object(json, ENTRY_LIST_SEND_DATE);
    if (send_date == -1) {
        return (EntryList) { .size = -1 };
    }
    // get read receipt
    bool read_receipt = (bool) get_int_from_object(json, ENTRY_LIST_READ_RECEIPT);
    if (read_receipt == -1) {
        return (EntryList) { .size = -1 };
    }
    EntryList entry_list = create_entry_list();
    entry_list_set_send_date(&entry_list, send_date);
    entry_list_set_recv_date(&entry_list, recv_date);
    entry_list_set_read_receipt(&entry_list, read_receipt);
    // Add entries array to EntryList
    cJSON *json_array = cJSON_GetObjectItemCaseSensitive(json, ENTRY_LIST_ENTRIES);
    if (json_array == NULL || !cJSON_IsArray(json_array)) {
        return (EntryList) { .size = -1 };
    }
    cJSON *string;
    cJSON_ArrayForEach(string, json_array) {
        char *entry = string->valuestring;
        if (!cJSON_IsString(string) || entry == NULL) {
            // Invalid string array element
            return (EntryList) { .size = -1 };
        }
        entry_list_push(&entry_list, entry);
    }
    return entry_list;
}

HistoryQueue json_to_history_queue(cJSON *json_array) {
    if (json_array == NULL || !cJSON_IsArray(json_array)) {
        return (HistoryQueue) { .size = -1 };
    }
    HistoryQueue history = create_history_queue();
    cJSON *json_obj;
    cJSON_ArrayForEach(json_obj, json_array) {
        EntryList next_entry_list = json_to_entry_list(json_obj);
        if (next_entry_list.size == -1) {
            return (HistoryQueue) { .size = -1 };
        }
        history_queue_push(&history, &next_entry_list);
    }
    return history;
}

cJSON *modulo_to_json(Modulo *modulo) {
    cJSON *json = cJSON_CreateObject(); 

    // add username to JSON
    if (cJSON_AddStringToObject(json, MODULO_USERNAME, modulo->username) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add wakeup_earliest to JSON
    if (cJSON_AddNumberToObject(json, MODULO_WAKEUP_EARLIEST, modulo->wakeup_earliest) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add wakeup_latest to JSON
    if (cJSON_AddNumberToObject(json, MODULO_WAKEUP_LATEST, modulo->wakeup_latest) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add entry_delimiter to JSON
    if (cJSON_AddStringToObject(json, MODULO_ENTRY_DELIMITER, modulo->entry_delimiter) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add day_ptr to JSON
    if (cJSON_AddNumberToObject(json, MOUDLO_DAY_PTR, modulo->day_ptr) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add today entries to JSON
    if (add_entry_list_to_object(json, MODULO_TODAY, &modulo->today) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add tomorrow entries to JSON
    if (add_entry_list_to_object(json, MODULO_TOMORROW, &modulo->tomorrow) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    // add history to JSON
    if (add_history_queue_to_object(json, MODULO_TOMORROW, &modulo->history) == NULL) {
        cJSON_Delete(json);
        return NULL;
    }

    return json;
}

cJSON *add_entry_list_to_object(cJSON *json, const char *name, EntryList *entry_list) {
    cJSON *json_entry_list = entry_list_to_json(entry_list);
    if (json_entry_list == NULL) {
        return NULL;
    }
    cJSON_AddItemToObject(json, name, json_entry_list);
    return json_entry_list;
}

cJSON *add_history_queue_to_object(cJSON *json, const char *name, HistoryQueue *history) {
    cJSON *json_history_queue = history_queue_to_json(history);
    if (json_history_queue == NULL) {
        return NULL;
    }
    cJSON_AddItemToObject(json, MODULO_HISTORY, json_history_queue);
    return json_history_queue;
}

cJSON *entry_list_to_json(EntryList *entry_list) {
    cJSON *json_obj = cJSON_CreateObject();

    if (cJSON_AddNumberToObject(json_obj, ENTRY_LIST_SEND_DATE, entry_list->send_date) == NULL) {
        return NULL;
    }

    if (cJSON_AddNumberToObject(json_obj, ENTRY_LIST_RECV_DATE, entry_list->recv_date) == NULL) {
        return NULL;
    }

    if (cJSON_AddNumberToObject(json_obj, ENTRY_LIST_READ_RECEIPT, entry_list->read_receipt) == NULL) {
        return NULL;
    }

    cJSON *json_array = cJSON_AddArrayToObject(json_obj, ENTRY_LIST_ENTRIES);
    char **entries = entry_list->entries;
    for (int i = 0; i < entry_list->size; i++) {
        char *entry = entry_list_get(entry_list, i);
        cJSON *string = cJSON_CreateString(entry);
        if (string == NULL) {
            // failed to create json string for entry
            return NULL;
        }
        cJSON_AddItemToArray(json_array, string);
    }
    return json_obj;

}

cJSON *history_queue_to_json(HistoryQueue *history) {
    cJSON *json_array = cJSON_CreateArray();

    for (int i = 0; i < history->size; i++) {
        EntryList *entry_list = history_queue_get(history, i);
        cJSON *json_entry_list = entry_list_to_json(entry_list);
        if (json_entry_list == NULL) {
            // failed to created entry list from history
            return NULL;
        }
        cJSON_AddItemToArray(json_array, json_entry_list);
    }
    return json_array;
}