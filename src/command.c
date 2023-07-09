#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "main.h"
#include "command.h"

void command_init(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulus init requires 0 additional command line args. %d supplied\n", argc-2);
    }
}

void command_set(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "modulus set requires 1 additional command line arg.\n");
        fprintf(stderr, "Try `modulus set username` or `modulus set wakeup`.\n");
        exit(-1);
    } else if (argc > 3) {
        fprintf(stderr, "modulus set requires 1 additional command line arg. %d supplied\n", argc-3);
        exit(-1);
    } 
    if (strcmp(argv[2], USERNAME) == 0) {
        command_set_name(argv[2]);
    } else if (strcmp(argv[2], WAKEUP) == 0) {
        command_set_wakeup(argv[2]);
    } else {
        fprintf(stderr, "Unrecognized arg '%s' supplied to modulus set.\n", argv[2]);
        fprintf(stderr, "Try `modulus set name` or `modulus set wakeup`.\n");
        exit(-1);
    }
}

void command_set_username(char *username) {
    Modulus *modulus = read_and_sync_data();

    char *prev_username = modulus->username;
    update_name(modulus, username);

    write_data(modulus);
    free(modulus);

    printf("Successfully updated username!\n");
    printf("Previous username: %s, New username: %s\n", prev_username, username);
}

void command_set_wakeup(char *wakeup) {
    Modulus *modulus = read_and_sync_data();

    char *prev_wakeup = modulus->wakeup;
    update_wakeup(modulus, wakeup);

    write_data(modulus);
    free(modulus);

    printf("Successfully updated wakeup!\n");
    printf("Previous wakeup: %s, New wakeup: %s\n", prev_wakeup, wakeup);
}

void command_tomorrow(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulus tomorrow requires 0 additional command line args. %d supplied\n", argc-2);
    }
}

void command_today(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulus today requires 0 additional command line args. %d supplied\n", argc-2);
    }

}

void command_peek(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "modulus today requires 0 additional command line args. %d supplied\n", argc-2);
    }

}

void command_remove(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "modulus remove requires 1 additional arg to specify the entry to remove.\n");
        fprintf(stderr, "Try `modulus remove 1` to remove the first entry from the tomorrow list");
    }
}

