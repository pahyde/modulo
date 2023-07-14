#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "command.h"
#include "filesystem.h"
#include "time_utils.h"
#include "modulo.h"

static int set_username(char *username, Modulo *modulo, OSContext *c);
static int set_wakeup(char *username, Modulo *modulo, OSContext *c);
static int set_entry_delimiter(char *username, Modulo *modulo, OSContext *c);

static bool length_ok(char *string, int max_length);

/*
    reads from disk (or initializes) a modulo struct. 
    Then syncs out-of-date data via modulo->wakeup time if necessary
    optionally persists 'changes' (initialization or synchronization) to disk
*/
static Modulo *load_updated_modulo(OSContext *c, bool write_updates_to_disk);

void command_root() {
    // display usage hints
    printf("Modulo is a minimal productivity app designed for continuity!\n");
    printf("It allows you to offload end-of-day thoughts, motivations, and goals onto tomorrows to-do list.\n"); 
    printf("\n"); 
    printf("Run `modulo set preferences` to setup your user preferences.\n");
    printf("Then run `modulo tomorrow` to start journaling your thoughts for tomorrow!\n\n");
}

void command_set_preferences() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_username(char *username) {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    if (set_username(username, modulo, c) == -1) {
        exit(1);
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_wakeup(char *wakeup) {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);

    //uint16_t prev_wakeup = modulo->wakeup;
    //update_wakeup(modulo, wakeup);
    //uint16_t new_wakeup = modulo->wakeup;

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);

    printf("Successfully updated wakeup!\n");
    //printf("Previous wakeup: %d, New wakeup: %s\n", format_time(prev_wakeup), format_time(new_wakeup));
}

void command_set_entry_delimiter(char *entry_delimiter) {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);

    if (set_entry_delimiter(entry_delimiter, modulo, c) == -1) {
        exit(1);
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_preferences() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_username() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);

    printf("Current username: %s\n", modulo_get_username(modulo));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_wakeup() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_entry_delimiter() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);

    printf("Current entry_delimiter: %s\n", modulo_get_entry_delimiter(modulo));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_tomorrow() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_today() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);
    char **today_entries = modulo->today.entries;
    free(modulo);
    free(c);
}

void command_peek() {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);
    free(modulo);
    free(c);
}

void command_remove(char *entry_number) {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

int set_username(char *username, Modulo *modulo, OSContext *c) {
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

    printf("Successfully updated username!\n");
    printf("Previous username: %s, New username: %s\n", prev_username, username);
    return 0;
}

int set_wakeup(char *wakeup, Modulo *modulo, OSContext *c) {
    time_t wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        fprintf(stderr, "Error parsing wakeup time: %s\n", wakeup);
        fprintf(stderr,"Your input must match one of the following formats:\n");
        fprintf(stderr,"AM/PM:\n");
        fprintf(stderr,"    1. %%H(am|pm)\n");
        fprintf(stderr,"    2. %%H:%%M(am|pm)\n");
        fprintf(stderr,"24-Hour:\n");
        fprintf(stderr,"    3. %%H\n");
        fprintf(stderr,"    4. %%H:%%M\n");
        fprintf(stderr,"\n");
        fprintf(stderr,"Note that white space and leading zeros are optional. Also matching is case insensitive.\n");
        fprintf(stderr,"i.e. '9am', '009:00 AM', '9:00am', and '9 : 00' are all valid.\n");
        return -1;
    }
    time_t prev_wakeup = modulo_get_wakeup(modulo);
    modulo_set_wakeup(modulo, wakeup_time);
    printf("Successfully updated wakeup!\n");
    printf("Previous wakeup: %s, New wakeup: %s\n", prev_wakeup, format_time(wakeup_time));
    return 0;
}

int set_entry_delimiter(char *entry_delimiter, Modulo *modulo, OSContext *c) {
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

    printf("Successfully updated entry_delimiter!\n");
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
Modulo *load_updated_modulo(OSContext *c, bool write_updates_to_disk) {
    Modulo *modulo = load_modulo(c);
    if (modulo != NULL) {
        if (write_updates_to_disk && !modulo_out_of_sync(modulo)) {
            // data is up to date so no need to write
            write_updates_to_disk = false;
        } else {
            // data may or may not be up-to-date
            // we're not writing to disk though so we just call sync
            // and skip an unnecessary out_of_sync check
            modulo_sync(modulo);
        }
    } else {
        char *username = get_system_username(c);
        modulo = create_default_modulo(username);
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
    Helper function to save modulo data
    If save fails, writes to stderr and exits
*/
void save_modulo_or_exit(Modulo *modulo, OSContext *c) {
    if (save_modulo(modulo, c) == -1) {
        char *filepath = c->modulo_json_filepath;
        fprintf(stderr, "Failure to save modulo data to %s\n", filepath);
    }
}
