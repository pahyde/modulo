#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "command.h"
#include "filesystem.h"
#include "time_utils.h"
#include "modulo.h"
#include "prompt.h"

static void command_set_wakeup_boundary(char *boundary, char *wakeup);

static int set_username(char *username, Modulo *modulo, OSContext *c, bool show_prev);
static int set_wakeup_earliest(char *wakeup, Modulo *modulo, bool show_prev);
static int set_wakeup_latest(char *wakeup, Modulo *modulo, bool show_prev);
static int set_entry_delimiter(char *username, Modulo *modulo, OSContext *c, bool show_prev);

static void print_init_msg();
static void print_wakeup_err_msg(char *wakeup);

static bool length_ok(char *string, int max_length);

/*
    reads from disk (or initializes) a modulo struct. 
    Then syncs out-of-date data via modulo->wakeup time if necessary
    optionally persists 'changes' (initialization or synchronization) to disk
*/
static Modulo *load_synced_modulo(OSContext *c, bool write_updates_to_disk);

void command_root() {
    // display usage hints
    printf("Modulo is a minimal productivity app designed for continuity!\n");
    printf("It allows you to offload end-of-day thoughts, motivations, and goals onto tomorrows to-do list.\n"); 
    printf("\n"); 
    printf("Run `modulo set preferences` to setup your user preferences.\n");
    printf("Then run `modulo tomorrow` to start journaling your thoughts for tomorrow!\n\n");
}

void command_init() {
    OSContext *c = get_context();
    char *username = get_system_username(c);
    Modulo *modulo = create_default_modulo(username);
    check_init(modulo);

    // print init message
    prompt_init_msg(username);

    // prompt earliest wakeup
    printf("What is the earliest you plan to wake up?\n");
    printf("(You can type 12-hour times like 9am, 9:30am or 24-hour times like 14:30)\n");
    prompt_set_wakeup_earliest(modulo, false);

    // prompt latest wakeup
    printf("What is the latest you plan to wakeup?\n");
    printf("Modulo will automatically advance to the next day after this time\n");
    prompt_set_wakeup_latest(modulo, false);
}

void command_set_preferences() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_username(char *username) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);
    if (set_username(username, modulo, c, true) == -1) {
        exit(1);
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_wakeup_earliest(char *wakeup) {
    command_set_wakeup_boundary(WAKEUP_BOUNDARY_EARLIEST, wakeup);
}

void command_set_wakeup_latest(char *wakeup) {
    command_set_wakeup_boundary(WAKEUP_BOUNDARY_LATEST, wakeup);
}

void command_set_wakeup_boundary(char *boundary, char *wakeup) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    if (strcmp(boundary, WAKEUP_BOUNDARY_EARLIEST) == 0) {
        set_wakeup_earliest(wakeup, modulo, true);
    } else {
        set_wakeup_latest(wakeup, modulo, true);
    }

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_entry_delimiter(char *entry_delimiter) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    if (set_entry_delimiter(entry_delimiter, modulo, c, true) == -1) {
        exit(1);
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_preferences() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_username() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    printf("Current username: %s\n", modulo_get_username(modulo));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_wakeup_earliest() {
    command_get_wakeup_boundary(WAKEUP_BOUNDARY_EARLIEST);
}

void command_get_wakeup_latest() {
    command_get_wakeup_boundary(WAKEUP_BOUNDARY_LATEST);
}

void command_get_wakeup_boundary(char *boundary) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    // wakeup time in minutes
    int *wakeup_time;
    if (strcmp(boundary, WAKEUP_BOUNDARY_EARLIEST) == 0) {
        wakeup_time = modulo_get_wakeup_earliest(modulo);
    } else {
        wakeup_time = modulo_get_wakeup_latest(modulo);
    }
    printf("Current wakeup_%s: %s\n", boundary, format_time(wakeup_time));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_entry_delimiter() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    printf("Current entry_delimiter: %s\n", modulo_get_entry_delimiter(modulo));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_tomorrow() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_today() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);
    char **today_entries = modulo->today.entries;
    free(modulo);
    free(c);
}

void command_wakeup() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);
    free(modulo);
    free(c);
}

void command_peek() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);
    free(modulo);
    free(c);
}

void command_history(char *selection) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);
    free(modulo);
    free(c);
}

void command_history_status() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);
    free(modulo);
    free(c);
}

void command_remove(char *entry_number) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

int set_username(char *username, Modulo *modulo, OSContext *c, bool show_prev) {
    if (!length_ok(username, USER_NAME_MAX_LEN)) {
        fprintf(
            stderr, 
            "Oops, the username \"%.15s...\" is too long! Usernames must be %d characters or less.\n",
            username,
            USER_NAME_MAX_LEN
        );
        return -1;
    }
    char prev_username[USER_NAME_MAX_LEN + 1];
    strcpy(prev_username, modulo_get_username(modulo));
    modulo_set_username(modulo, username);

    printf("Successfully set username!\n");
    if (show_prev) {
        printf("Previous username: %s, ", prev_username);
    }
    printf("New username: %s\n", username);
    return 0;
}

int set_wakeup_earliest(char *wakeup, Modulo *modulo, bool show_prev) {
    int wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        print_wakeup_err_msg(wakeup);
        return -1;
    }
    time_t prev_wakeup = modulo_get_wakeup_earliest(modulo);
    modulo_set_wakeup_earliest(modulo, wakeup_time);
    printf("Successfully set earliest wakeup to %s!\n", format_time(wakeup_time));
    if (show_prev) {
        printf("Previous wakeup_earliest: %s\n", prev_wakeup);
    }
    return 0;
}

int set_wakeup_latest(char *wakeup, Modulo *modulo, bool show_prev) {
    int wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        print_wakeup_err_msg(wakeup);
        return -1;
    }
    time_t prev_wakeup = modulo_get_wakeup_latest(modulo);
    modulo_set_wakeup_latest(modulo, wakeup_time);
    printf("Successfully set latest wakeup to %s!\n", format_time(wakeup_time));
    if (show_prev) {
        printf("Previous wakeup_latest: %s\n", prev_wakeup);
    }
    return 0;
}

int set_wakeup_recent(char *wakeup, Modulo *modulo) {
    int wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        print_wakeup_err_msg(wakeup);
        return -1;
    }
    printf("Successfully set latest wakeup to %s!\n", format_time(wakeup_time));
    if (show_prev) {
        printf("Previous wakeup_latest: %s\n", prev_wakeup);
    }
    return 0;
}

int set_entry_delimiter(char *entry_delimiter, Modulo *modulo, OSContext *c, bool show_prev) {
    if (!length_ok(entry_delimiter, DELIMITER_MAX_LEN)) {
        fprintf(
            stderr, 
            "Oops, the delimiter \"%.7s...\" is too long! Entry delimiter must be %d characters or less.\n",
            entry_delimiter,
            DELIMITER_MAX_LEN
        );
        return -1;
    }
    char prev_entry_delimiter[DELIMITER_MAX_LEN+1];
    strcpy(prev_entry_delimiter, modulo_get_entry_delimiter(modulo));
    modulo_set_entry_delimiter(modulo, entry_delimiter);

    printf("Successfully set entry_delimiter!\n");
    if (show_prev) {
        printf("Previous entry_delimiter: %s, ", prev_entry_delimiter);
    }
    printf("Previous entry_delimiter: %s, New entry_delimiter: %s\n", prev_entry_delimiter, entry_delimiter);
}

bool length_ok(char *string, int max_length) {
    return strnlen(string, max_length+1) <= max_length;
}

/*
    Lesson: 
    It's common to write one body of code A to serve the functionality
    of another body of code B

    It's tempting to write A in a way that caters to the needs of B. 
    This is inflexible when we introduce C - a new consumer of A

    It's better to write convenience functions in B or C that wrap 
    atomic and composable functions written in A
*/


/*
    Helper function for modulo commands 
    Loads modulo data from disk if it exists
    Otherwise the data is created and initialized with username env variable
    The modulo data is then synchronized via modulo->last_updated.

    If write_updates_to_disk is true, the resulting modulo struct is written to modulo.json in user's config dir
    Finally the modulo struct is returned to the calling function
*/
Modulo *load_synced_modulo(OSContext *c, bool write_updates_to_disk) {
    Modulo *modulo = load_modulo(c);
    if (modulo == NULL) {
        return NULL;
    } 
    bool sync_occurred = modulo_check_sync(modulo);
    if (!sync_occurred) {
        // Modulo exists and is already synced (disk is already up to date)
        write_updates_to_disk = false;
    }
    if (write_updates_to_disk && save_modulo(modulo, c) == -1) {
        char *filepath = c->modulo_json_filepath;
        fprintf(stderr, "Failed to sync modulo data with disk\n");
        fprintf(stderr, "Error occured while attempting to write to %s\n", filepath);
        exit(EXIT_FAILURE);
    }
    return modulo;
}

/*
    Helper function to check if modulo is initialized 
    If not, prompt user to run `modulo init` and exit
*/
void check_init(Modulo *modulo) {
    if (modulo == NULL) {
        printf("Modulo is uninitialized!\n");
        printf("run `modulo init` to configure Modulo to sync with your schedule.\n");
        exit(EXIT_FAILURE);
    }
}

/*
    Helper function to save modulo data
    If save fails, writes to stderr and exits
*/
void save_modulo_or_exit(Modulo *modulo, OSContext *c) {
    if (save_modulo(modulo, c) == -1) {
        char *filepath = c->modulo_json_filepath;
        fprintf(stderr, "Failure to save modulo data to %s\n", filepath);
    }
}
