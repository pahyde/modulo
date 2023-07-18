#include <stdio.h>

#include "cli.h"

static void print_wakeup_err_msg(char *wakeup);

void prompt_init_message(char *username) {
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

void prompt_set_day_ptr(int wakeup_earliest, int wakeup_latest) {

}

void prompt_set_username(Modulo *modulo, bool show_prev) {

}

void prompt_set_wakeup_earliest(Modulo *modulo, bool show_prev) {
    char wakeup_earliest[32];
    do {
        printf("\n");
        printf("earliest wakeup: ");
        fgets(wakeup_earliest, sizeof wakeup_earliest, stdin);
        printf("\n");
    } while (prompt_set_wakeup_earliest_once(wakeup_earliest, modulo, false) == -1);
}

void prompt_set_wakeup_latest(Modulo *modulo, bool show_prev) {
    char wakeup_latest[32];
    do {
        printf("\n");
        printf("latest wakeup: ");
        fgets(wakeup_latest, sizeof wakeup_latest, stdin);
        printf("\n");
    } while (set_wakeup_latest(wakeup_latest, modulo, false) == -1);

}
void prompt_set_entry_delimiter(Modulo *modulo, bool show_prev) {

}

int prompt_set_username_once(Modulo *modulo, char *username, bool show_prev) {
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

int prompt_set_wakeup_earliest_once(Modulo *modulo, char *wakeup, bool show_prev) {

}
int prompt_set_wakeup_latest_once(Modulo *modulo, char *wakeup, bool show_prev) {

}
int prompt_set_entry_delimiter_once(Modulo *modulo, char *entry_delimiter, bool show_prev) {

}

void print_wakeup_err_msg(char *wakeup) {
    fprintf(stderr, "Error parsing wakeup time: %s\n", wakeup);
    fprintf(stderr,"Your input must match one of the following formats:\n");
    fprintf(stderr,"AM/PM:   1. %%H(am|pm) 2. %%H:%%M(am|pm)\n");
    fprintf(stderr,"24-Hour: 3. %%H        4. %%H:%%M\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Note that white space and leading zeros are optional. Also matching is case insensitive.\n");
    fprintf(stderr,"i.e. '9am', '009:00 AM', '9:00am', and '9 : 00' are all valid.\n");
}