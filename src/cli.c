#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "cli.h"
#include "time_utils.h"
#include "command.h"


/*
micro-lesson: EOF is only meaningful to functions like fgetc, getchar, fgets at line start
*/

static FsmState next_state(FsmState curr, char c);

static int  cli_get_input_token(char *input_buffer, size_t max_input_length);
static void cli_gets(char *buffer, size_t buf_size);
static void clear_stdin();

static void cli_print_wakeup_error_message(char *wakeup);

static char *wakeup_range_to_string(Modulo *modulo);

static void string_tolower(char *str);
static bool length_ok(char *string, int max_length);


void cli_print_init_hello(char *username) {
    printf("Welcome! Modulo is a productivity app built to bridge the ");
    printf("gap between today's thoughts and tomorrow's actions!\n");
    printf("\n");
    printf("It's like a personal messaging system that let's you\n");
    printf("    1. send thoughts to tomorrow\n");
    printf("    2. read thoughts from yesterday.\n");
    printf("\n");
    printf("To get started, Modulo will need some information ");
    printf("to sync to your schedule.\n");
}

void cli_print_init_goodbye(Modulo *modulo) {
    printf("\n");
    printf("Awesome, you're all set up! Start sending messages to tomorrow's inbox with the `modulo tomorrow` command.\n");
    int wakeup_latest = modulo->wakeup_latest;
    printf_time("If you wake up before %s (your latest wakeup), run the `modulo wakeup` command.\n", wakeup_latest);
    printf("Then run `modulo today` to see the messages!\n");
    printf_time("Alternatively, you can omit the `modulo wakeup` command if you wake up after %s.\n", wakeup_latest);
}

void cli_print_preferences(Modulo *modulo) {
    printf("The following preferences are set:\n");
    printf("    1. username: %s\n", modulo->username);
    printf_time("    2. wakeup_earliest: %s\n", modulo->wakeup_earliest);
    printf_time("    3. wakeup_latest: %s\n", modulo->wakeup_latest);
    printf("    4. entry_delimiter: %s\n", modulo->entry_delimiter);
}

void cli_print_wakeup_success(Modulo *modulo) {
    printf("------------------------------------------\n");
    printf("Good morning %s!\n\n", modulo->username);
    cli_print_time_status(modulo);
    printf("\n");
    int new_entries = modulo->today.size;
    if (new_entries > 0) {
        printf("You have %d new entries to review today!\n", new_entries);
        printf("Run `modulo today` to view them or run `modulo tomorrow` to start journaling your thoughts for tomorrow.\n");
    } else {
        printf("No entries to review today -- You have to start somewhere!\n");
        printf("Run `modulo tomorrow` to start journaling your thoughts for tomorrow.\n");
    }
}

void cli_print_wakeup_failure(Modulo *modulo) {
    printf("Your next wakeup range is scheduled for %s\n", wakeup_range_to_string(modulo));
    printf_time("The current time %s is too early\n", utc_to_time(utc_now()));
    printf("\nRun `modulo set wakeup_earliest` or `modulo set preferences` to configure your wakeup range\n");
}

void cli_prompt_day_ptr(Modulo *modulo, time_t recent_wakeup_earliest, time_t recent_wakeup_latest) {
    time_t now = time(NULL);
    int time_minutes = utc_to_time(now);
    printf_time(
        "The current time %s is between your wakeup range.\nCan we assume you're up for a new day?\n", 
        time_minutes
    );
    bool is_yes = cli_prompt_yes_or_no();
    time_t day_ptr_0;
    if (is_yes) {
        /*
        Quick note:
        It may be more reliable to pass in recent and next wakup latest. It's "unbelievably" unlikely
        that next_wakeup_latest could refer to 24 hours from now as an edge case. But it is, you know,... possible.

        How? If the calling code occurs right before wakeup latest, and this code runs right after. 
        I doubt there will ever be 2 seconds of latency between these two lines practically speaking.
        */
        time_t next_wakeup_latest = time_to_utc_next(modulo->wakeup_latest, recent_wakeup_earliest);
        // start new day
        day_ptr_0 = next_wakeup_latest;
    } else {
        // remain in current day
        day_ptr_0 = recent_wakeup_latest;
    }
    modulo_set_day_ptr(modulo, day_ptr_0);
}

Selection cli_prompt_preference_selection() {
    char input[MAX_INPUT_LENGTH+1];
    while (true) {
        cli_prompt_input_token(
            "Enter a number to set a preference: ", 
            input, 
            MAX_INPUT_LENGTH
        );
        string_tolower(input);
        if (strcmp(input, CLI_DONE) == 0) {
            return DONE;
        } else if (strlen(input) != 1) {
            fprintf(stderr, "Bad input: %s.\n", input);
            fprintf(stderr, "Pick a numer in the range 1-4 or type done.\n");
            continue;
        }
        int item_number = atoi(input);
        switch (item_number) {
            case 1:
                return PREFERENCE_USERNAME;
            case 2:
                return PREFERENCE_WAKEUP_EARLIEST;
            case 3:
                return PREFERENCE_WAKEUP_LATEST;
            case 4:
                return PREFERENCE_ENTRY_DELIMITER;
            default:
                printf("%s is not a valid preference selection. Pick a number in the range 1-4.\n", input);
        }
    }
}

void cli_prompt_username(Modulo *modulo, bool show_prev) {
    char username[MAX_INPUT_LENGTH+1];
    do {
        cli_prompt_input_token(
            "username: ", 
            username, 
            MAX_INPUT_LENGTH
        );
        printf("\n");
    } while (cli_set_username(modulo, username, true) == -1);
}

void cli_prompt_wakeup_earliest(Modulo *modulo, bool show_prev) {
    char wakeup_earliest[32];
    do {
        printf("\n");
        printf("earliest wakeup: ");
        cli_gets(wakeup_earliest, sizeof wakeup_earliest);
        printf("\n");
    } while (cli_set_wakeup_earliest(modulo, wakeup_earliest, false) == -1);
}

void cli_prompt_wakeup_latest(Modulo *modulo, bool show_prev) {
    char wakeup_latest[32];
    do {
        printf("\n");
        printf("latest wakeup: ");
        cli_gets(wakeup_latest, sizeof wakeup_latest);
        printf("\n");
    } while (cli_set_wakeup_latest(modulo, wakeup_latest, false) == -1);
}

void cli_prompt_entry_delimiter(Modulo *modulo, bool show_prev) {
    char entry_delimiter[MAX_INPUT_LENGTH+1];
    do {
        cli_prompt_input_token(
            "entry_delimiter: ", 
            entry_delimiter, 
            MAX_INPUT_LENGTH
        );
        printf("\n");
    } while (cli_set_entry_delimiter(modulo, entry_delimiter, true));
}

void cli_prompt_input_token(char *prompt, char *input_buffer, size_t max_input_length) {
    do {
        printf("\n");
        printf(prompt);
    } while (cli_get_input_token(input_buffer, max_input_length) == -1);
}

int cli_set_username(Modulo *modulo, char *username, bool show_prev) {
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

    printf("Successfully updated username to %s!\n", username);
    if (show_prev) {
        printf("Previous username: %s\n", prev_username);
    }
    return 0;
}

int cli_set_wakeup_earliest(Modulo *modulo, char *wakeup, bool show_prev) {
    int wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        cli_print_wakeup_error_message(wakeup);
        return -1;
    }
    int prev_wakeup = modulo_get_wakeup_earliest(modulo);
    modulo_set_wakeup_earliest(modulo, wakeup_time);
    printf_time("Successfully set earliest wakeup to %s!`\n", wakeup_time);
    if (show_prev) {
        printf_time("Previous wakeup_earliest: %s\n", prev_wakeup);
    }
    return 0;
}

int cli_set_wakeup_latest(Modulo *modulo, char *wakeup, bool show_prev) {
    int wakeup_time;
    if ((wakeup_time = parse_time(wakeup)) == -1) {
        cli_print_wakeup_error_message(wakeup);
        return -1;
    }
    int prev_wakeup = modulo_get_wakeup_latest(modulo);
    modulo_set_wakeup_latest(modulo, wakeup_time);
    printf_time("Successfully set latest wakeup to %s!\n", wakeup_time);
    if (show_prev) {
        printf_time("Previous wakeup_latest: %s\n", prev_wakeup);
    }
    return 0;
}

int cli_set_entry_delimiter(Modulo *modulo, char *entry_delimiter, bool show_prev) {
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

    printf("Successfully updated entry_delimiter to %s!\n", entry_delimiter);
    if (show_prev) {
        printf("Previous entry_delimiter: %s\n", prev_entry_delimiter);
    }
    return 0;
}

int cli_get_input_token(char *input_buffer, size_t max_input_length) {
    size_t buf_idx = 0;
    char c;
    FsmState curr = START;

    /*
    valid_input: bool
    true if the input is either
        1. a single non space-delimited token
        2. a multi-token enclosed in double-quotes
    */
    bool valid_input = true;
    while ((c = getchar()) != '\n' && c != EOF) {
        FsmState next = next_state(curr, c);  
        if (next == INVALID) {
            valid_input = false;
            // clear stdin
            clear_stdin();
            break;
        } 
        bool is_char_literal = false;
        if (curr == START && next == RAW) {
            is_char_literal = true;
        } else if (curr == RAW && next == RAW) {
            is_char_literal = true;     
        } else if (curr == QUOTED && next == QUOTED) {
            is_char_literal = true;     
        } else if (curr == RAW_ESCAPE && next == RAW) {
            is_char_literal = true;     
        } else if (curr == QUOTED_ESCAPE && next == QUOTED) {
            is_char_literal = true;     
        }
        if (is_char_literal) {
            if (buf_idx == max_input_length) {
                // input too long
                input_buffer[buf_idx] = '\0';
                clear_stdin();
                fprintf(stderr, "Provided input \"%.7s...\" is too long.\n");
                return -1;
            }
            input_buffer[buf_idx++] = c;
        }
        curr = next;
    }
    if (curr != RAW && curr != SPACE) {
        valid_input = false;
    }
    if (!valid_input) {
        // invalid string format
        fprintf(stderr, "Invalid input format!\nYour input should be a single token. Use double-quotes to submit a multi-word input.\n");
        return -1;
    }
    input_buffer[buf_idx] = '\0';
    return 0;
}

FsmState next_state(FsmState curr, char c) {
    switch (curr) {
        case START:
            if (c == '\s') return START;
            if (c == '\"') return QUOTED;
            return RAW;
        case RAW:
            if (c == '\s') return INVALID;
            if (c == '\\') return RAW_ESCAPE;
            if (c == '\"') return QUOTED;
            return RAW;
        case RAW_ESCAPE:
            return RAW;
        case QUOTED:
            if (c == '\\') return QUOTED_ESCAPE;
            if (c == '\"') return RAW;
            return QUOTED;
        case QUOTED_ESCAPE:
            return QUOTED;
        case SPACE:
            if (c == '\s') return SPACE;
            return INVALID;
        default:
            fprintf(stderr, "Invalid or unrecognized FSM state. No next state for %d\n", curr);
            exit(EXIT_FAILURE);
    }
}

bool length_ok(char *string, int max_length) {
    return strnlen(string, max_length+1) <= max_length;
}


bool cli_prompt_yes_or_no() {
    char response[4];
    while (true) {
        printf("\n");
        printf("(yes or no): ");
        cli_gets(response, sizeof response);
        string_tolower(response);
        if (strcmp(response, "yes") == 0) {
            return true;
        } else if (strcmp(response, "no") == 0) {
            return false; 
        }
        printf("Unrecognized response %s. You can type yes or no (case insensitive)\n");
    }
}

void string_tolower(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        str[i] = tolower(str[i]);
    }
}

void cli_gets(char *buffer, size_t buf_size) {
    if (fgets(buffer, buf_size, stdin) == NULL) {
        fprintf(stderr, "Error reading from stdin\n");
        exit(EXIT_FAILURE);
    }
    char *new_line = strchr(buffer, '\n');
    if (new_line == NULL) {
        clear_stdin();
    } else {
        *new_line = '\0';
    }
}

void clear_stdin() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void cli_print_wakeup_error_message(char *wakeup) {
    fprintf(stderr, "Error parsing wakeup time: %s\n", wakeup);
    fprintf(stderr,"Your input must match one of the following formats:\n");
    fprintf(stderr,"AM/PM:   1. %%H(am|pm) 2. %%H:%%M(am|pm)\n");
    fprintf(stderr,"24-Hour: 3. %%H        4. %%H:%%M\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Note that white space and leading zeros are optional. Also matching is case insensitive.\n");
    fprintf(stderr,"i.e. '9am', '009:00 AM', '9:00am', and '9 : 00' are all valid.\n");
}

void cli_print_time_status(Modulo *modulo) {
    printf("It's currently %s.\n", utc_to_string(utc_now(), false));
    printf("Your next wakeup is scheduled for %s.\n", wakeup_range_to_string(modulo));
}

void cli_print_entry_lists_status(Modulo *modulo) {
    printf("Entry List Status\n");
    printf("-----------------\n");
    printf("today    (inbox):  %d entries to review today\n", modulo->today.size);
    printf("tomorrow (outbox): %d entries written for tomorrow\n", modulo->tomorrow.size);
    printf("history: \n");
    for (size_t i = 0; i < HISTORY_QUEUE_LENGTH; i++) {
        if (i < modulo->history.size) {
            EntryList *entry_list = &modulo->history.entry_lists[i];
            printf("    %d. send date: %s\n", i+1, utc_to_string(entry_list->send_date, false));
        } else {
            printf("    %d. -\n", i+1);
        }
    }
}

char *wakeup_range_to_string(Modulo *modulo) {
    time_t day_ptr = modulo->day_ptr;
    time_t next_wakeup_earliest = time_to_utc_next(modulo->wakeup_earliest, day_ptr);
    time_t next_wakeup_latest = time_to_utc_next(modulo->wakeup_latest, next_wakeup_earliest);
    return utc_range_to_string(next_wakeup_earliest, next_wakeup_latest);
}