#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "command.h"
#include "filesystem.h"
#include "time_utils.h"
#include "modulo.h"
#include "cli.h"
#include "editor/entry_editor.h"

static void command_set_wakeup_boundary(char *boundary, char *wakeup);

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
    cli_print_init_hello(username);

    // prompt earliest wakeup
    printf("What is the earliest you plan to wake up?\n");
    printf("(You can type 12-hour times like 9am, 9:30am or 24-hour times like 14:30)\n");
    cli_prompt_wakeup_earliest(modulo, false);

    // prompt latest wakeup
    printf("What is the latest you plan to wakeup?\n");
    printf("Modulo will automatically advance to the next day after this time\n");
    cli_prompt_wakeup_latest(modulo, false);

    time_t recent_wakeup_earliest = time_to_utc_prev(modulo->wakeup_earliest, utc_now());
    time_t recent_wakeup_latest = time_to_utc_prev(modulo->wakeup_latest, utc_now());
    if (recent_wakeup_latest < recent_wakeup_earliest) {
        // wakeup_latest < wakeup_earliest < now < wakeup_latest
        cli_prompt_day_ptr(modulo, recent_wakeup_earliest, recent_wakeup_latest);
    } else {
        modulo_set_day_ptr(modulo, recent_wakeup_latest);
    }
    cli_print_init_goodbye(modulo);
    // clean up
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_preferences() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    printf("This utility helps you update your user preferences.\nType `done` at any time to abort\n\n");

    bool done = false;
    while (!done) {
        cli_print_preferences(modulo);
        Selection selection = cli_prompt_preference_selection();
        switch (selection) {
            case PREFERENCE_USERNAME:
                cli_prompt_username(modulo, true);
                break;
            case PREFERENCE_WAKEUP_EARLIEST:
                cli_prompt_wakeup_earliest(modulo, true);
                break;
            case PREFERENCE_WAKEUP_LATEST:
                cli_prompt_wakeup_latest(modulo, true);
                break;
            case PREFERENCE_ENTRY_DELIMITER:
                cli_prompt_entry_delimiter(modulo, true);
                break;
            case DONE:
                done = true;
                break;
            default:
                fprintf(stderr, "Error: Unrecognized preference selection %d\n", selection);
                exit(EXIT_FAILURE);
        }
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_username(char *username) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);
    if (cli_set_username(modulo, username, true) == -1) {
        exit(EXIT_FAILURE);
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
        cli_set_wakeup_earliest(modulo, wakeup, true);
    } else {
        cli_set_wakeup_latest(modulo, wakeup, true);
    }

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_set_entry_delimiter(char *entry_delimiter) {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    if (cli_set_entry_delimiter(entry_delimiter, modulo, true) == -1) {
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

    cli_print_preferences(modulo);

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
    printf("Current wakeup_%s: %s\n", boundary, time_to_string(wakeup_time));

    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void command_get_entry_delimiter() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    printf("Current entry_delimiter: %s\n", modulo_get_entry_delimiter(modulo));

    free(modulo);
    free(c);
}

void command_status() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    printf("------------------------------------------\n");
    cli_print_time_status(modulo);
    printf("\n");
    cli_print_entry_lists_status(modulo);
    printf("\n");

    free(modulo);
    free(c);
}

void command_tomorrow() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    entry_editor_start(modulo, c);

    free(modulo);
    free(c);
}

void command_today() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    cli_print_today_entries(modulo);  

    free(modulo);
    free(c);
}

/* 
micro lesson: we can prove that
n / m / m = n / (m*m) for ints n,m
*/
void command_wakeup() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, false);
    check_init(modulo);

    // times quoted in seconds from day_ptr
    offset_t now = utc_to_offset(modulo, utc_now());
    offset_t wakeup_earliest = time_to_offset(modulo, modulo->wakeup_earliest);

    // hours until wakeup earliest
    int minutes_until_next_wakeup = (wakeup_earliest - now) / 60;
    if (minutes_until_next_wakeup <= 0) {
        wakeup_success(modulo);
    } else if (minutes_until_next_wakeup <= 2*60) {
        printf_time("The current time %s is pretty early for your usual wakeup range:\n", utc_to_time(time(NULL)));
        printf_time("%s - ", modulo->wakeup_earliest);
        printf_time("%s\n\n", modulo->wakeup_latest);
        printf("Are you sure you want to wakeup?\n\n");
        bool is_yes = cli_prompt_yes_or_no();
        if (is_yes) {
            wakeup_success(modulo);
        } else {
            wakeup_failure(modulo);
        }
    } else {
        wakeup_failure(modulo);
    }
    save_modulo_or_exit(modulo, c);
    free(modulo);
    free(c);
}

void wakeup_success(Modulo *modulo) {
    modulo_sync_forward(modulo, 1);
    cli_print_wakeup_success(modulo);
}

void wakeup_failure(Modulo *modulo) {
    cli_print_wakeup_failure(modulo);
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

    // parse history item number
    int item_number;
    sscanf(selection, "%d", &item_number);

    HistoryQueue *history = &modulo->history;
    uint8_t size = history->size;
    if (size == 0) {
        printf("Your history queue is empty!\n", size);
        printf("Come back after you've used modulo a bit longer!\n");
    } else if (item_number > size || item_number < 1) {
        printf("You have %d old entry lists saved to your history queue.\n", size);
        printf("Can't get item number: %d\n", item_number);
    } else {
        int index = item_number-1;
        cli_print_history_item(history, index);
    }

    free(modulo);
    free(c);
}

void command_history_status() {
    OSContext *c = get_context();
    Modulo *modulo = load_synced_modulo(c, true);
    check_init(modulo);

    cli_print_history_status(&modulo->history);

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
