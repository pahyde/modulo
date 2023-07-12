#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "main.h"
#include "command.h"
#include "filesystem.h"
#include "actions.h"
#include "util.h"

/*
    reads from disk (or initializes) a modulo struct. 
    Then syncs out-of-date data via modulo->wakeup time if necessary
    optionally persists changes (initialization or synchronization) to disk
*/
static Modulo *load_updated_modulo(OSContext *c, bool write_updates_to_disk);

void command_root() {
    // display usage hints
    printf("Modulo is a minimal productivity app designed for continuity!\n");
    printf("It allows you to offload end-of-day thoughts, motivations, and goals onto tomorrows to-do list.\n"); 
    printf("\n"); 
    printf("Run `modulo init` to setup your user preferences.\n");
    printf("Then run `modulo tomorrow` to start recording your thoughts for tomorrow!\n\n");
}

/*
    tentative TODO: remove this command (replaced with modulo set preferences)
*/
void command_init(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulo init requires 0 additional command line args. %d supplied\n", argc-2);
    }
}

// TODO: set positional arguments = argc - 1
// better readability
void command_set(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "modulo set requires 1 additional command line arg.\n");
        fprintf(stderr, "Try `modulo set username` or `modulo set wakeup`.\n");
        exit(-1);
    } else if (argc > 4) {
        fprintf(stderr, "modulo set requires 1 additional command line arg. %d supplied\n", argc-3);
        exit(-1);
    } 
    if (strcmp(argv[2], USERNAME) == 0) {
        command_set_username(argv[3]);
    } else if (strcmp(argv[2], WAKEUP) == 0) {
        command_set_wakeup(argv[3]);
    } else {
        fprintf(stderr, "Unrecognized arg '%s' supplied to modulo set.\n", argv[2]);
        fprintf(stderr, "Try `modulo set name` or `modulo set wakeup`.\n");
        exit(-1);
    }
}

void command_set_username(char *username) {
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    char *prev_username = modulo->username;
    update_username(modulo, username);

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);

    printf("Successfully updated username!\n");
    printf("Previous username: %s, New username: %s\n", prev_username, username);
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

    //printf("Successfully updated wakeup!\n");
    //printf("Previous wakeup: %d, New wakeup: %s\n", format_time(prev_wakeup), format_time(new_wakeup));
}

void command_tomorrow(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulo tomorrow requires 0 additional command line args. %d supplied\n", argc-2);
    }
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_today(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulo today requires 0 additional command line args. %d supplied\n", argc-2);
    }
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);
    char **today_entries = modulo->today.entries;
    free(modulo);
    free(c);
}

void command_peek(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulo today requires 0 additional command line args. %d supplied\n", argc-2);
    }
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, true);
    free(modulo);
    free(c);
}

void command_remove(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "modulo remove requires 1 additional arg to specify the entry to remove.\n");
        fprintf(stderr, "Try `modulo remove 1` to remove the first entry from the tomorrow list");
    }
    OSContext *c = get_context();
    Modulo *modulo = load_updated_modulo(c, false);
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
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
        if (write_updates_to_disk && !out_of_sync(modulo)) {
            // data is up to date so no need to write
            write_updates_to_disk = false;
        } else {
            // data may or may not be up-to-date
            // we're not writing to disk though so we just call sync
            // and skip an unnecessary out_of_sync check
            sync(modulo);
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
