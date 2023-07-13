#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modulo.h"
#include "command_router.h"
#include "json.h"
#include "modulo.h"


static void route_set(int argc, char **argv);
static void route_set_preferences(int argc, char **argv);
static void route_set_preference(int argc, char **argv);

static void route_get(int argc, char **argv);
static void route_get_preferences(int argc, char **argv);
static void route_get_preference(int argc, char **argv);

static void route_tomorrow(int argc, char **argv);
static void route_today(int argc, char **argv);
static void route_peek(int argc, char **argv);

static void route_remove(int argc, char **argv);

static void check_argc(int argc, char **argv, int sub_cmds, int args);
static void unknown_sub_command(char **argv, char *sub_cmd, int parent_cmds);

/*
    Lesson: separation of concerns

    chronological and logical separation vs Physical (memory vs disk vs network) separation

    chronological: main -> router -> command
    physical: filesystem.c (disk) vs action.c (mem)

*/

void route_command(int argc, char **argv) {
    if (argc == 1) {
        command_root();
        return 0;
    }
    char *sub_cmd = argv[1];
    if (strcmp(sub_cmd, COMMAND_SET) == 0) {
        route_set(argc, argv);
    } else if (strcmp(sub_cmd, COMMAND_GET) == 0) {
        route_get(argc, argv);
    } else if (strcmp(sub_cmd, COMMAND_TOMORROW) == 0) {
        route_tomorrow(argc, argv);
    } else if (strcmp(sub_cmd, COMMAND_TODAY) == 0) {
        route_today(argc, argv);
    } else if (strcmp(sub_cmd, COMMAND_PEEK) == 0) {
        route_peek(argc, argv);
    } else if (strcmp(sub_cmd, COMMAND_REMOVE) == 0) {
        route_remove(argc, argv);
    } else {
        int parent_cmds = 0;
        unknown_sub_command(argv, sub_cmd, parent_cmds);
    }
}

void route_set(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "modulo set requires a subcommand.\n");
        fprintf(stderr, "Try `modulo set preferences` to set all preferences`.\n");
        exit(1);
    }
    char *sub_cmd = argv[2];
    if (strcmp(sub_cmd, COMMAND_PREFERENCES) == 0) {
        route_set_preferences(argc, argv);
    } else {
        route_set_preference(argc, argv);
    }
}

void route_set_preferences(int argc, char **argv) {
    int sub_cmds = 2;
    int args = 0;
    check_argc(argc, argv, sub_cmds, args);
    command_set_preferences();
}

void route_set_preference(int argc, char **argv) {
    char *sub_cmd = argv[2];
    int sub_cmds = 2;
    int args = 1;
    if (strcmp(sub_cmd, COMMAND_USERNAME) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_set_username(argv[3]);
    } else if (strcmp(sub_cmd, COMMAND_WAKEUP) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_set_wakeup(argv[3]);
    } else if (strcmp(sub_cmd, COMMAND_ENTRY_DELIMITER) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_set_entry_delimiter(argv[3]);
    } else {
        int parent_cmds = 1;
        unknown_sub_command(argv, sub_cmd, parent_cmds);
    }
}

void route_get(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "modulo get requires a subcommand.\n");
        fprintf(stderr, "Try `modulo get preferences` to see your current preferences`.\n");
        exit(1);
    }
    char *sub_cmd = argv[2];
    if (strcmp(sub_cmd, COMMAND_PREFERENCES) == 0) {
        route_get_preferences(argc, argv);
    } else {
        route_get_preference(argc, argv);
    }
}

void route_get_preferences(int argc, char **argv) {
    int sub_cmds = 2;
    int args = 0;
    check_argc(argc, argv, sub_cmds, args);
    command_set_preferences();
}

void route_get_preference(int argc, char **argv) {
    char *sub_cmd = argv[2];
    int sub_cmds = 2;
    int args = 0;
    if (strcmp(sub_cmd, COMMAND_USERNAME) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_get_username();
    } else if (strcmp(sub_cmd, COMMAND_WAKEUP) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_get_wakeup();
    } else if (strcmp(sub_cmd, COMMAND_ENTRY_DELIMITER) == 0) {
        check_argc(argc, argv, sub_cmds, args);
        command_get_entry_delimiter();
    } else {
        int parent_cmds = 1;
        unknown_sub_command(argv, sub_cmd, parent_cmds);
    }
}

void route_tomorrow(int argc, char **argv) {
    int sub_cmds = 1;
    int args = 0;
    check_argc(argc, argv, sub_cmds, args);
    command_tomorrow(argc, argv);
}

void route_today(int argc, char **argv) {
    int sub_cmds = 1;
    int args = 0;
    check_argc(argc, argv, sub_cmds, args);
    command_today();
}

void route_peek(int argc, char **argv) {
    int sub_cmds = 1;
    int args = 0;
    check_argc(argc, argv, sub_cmds, args);
    command_peek();
}

void route_remove(int argc, char **argv) {
    int sub_cmds = 1;
    int args = 1;
    check_argc(argc, argv, sub_cmds, args);
    char *entry_number = argv[2];
    command_remove(entry_number);
}

void unknown_sub_command(char **argv, char *sub_cmd, int parent_cmds) {
    fprintf(stderr, "Error: unknown command \"%s\" for \"", sub_cmd);
    fprintf(stderr, "modulo");
    for (int i = 0; i < parent_cmds; i++) {
        fprintf(stderr, " %s", argv[i+1]);
    }
    fprintf(stderr, "\"\n");
    exit(1);
}

/* 
    Helper function to check if a command has the correct number of positional args
    Guaranteed that the calling function has validated the sub commands

    i.e. for the command `modulo set username night_owl`
    the calling function has already validated 'modulo set username'
 */
void check_argc(int argc, char **argv, int sub_cmds, int expected_args) {
    // TODO: return -1 for error and remove exit()
    // this way commands can add custom error messages if desired
    int actual_args = argc - (1+sub_cmds);
    if (actual_args < expected_args) {
        // not enough positional args
        fprintf(stderr, "Error: not enough positional arguments for command ");
        print_cmd_stderr(argv, sub_cmds); 
        fprintf(stderr, "expected arg count: %d, actual arg count: %d\n", expected_args, actual_args);
        exit(1);
    }
    if (actual_args > expected_args) {
        // too many positional args
        fprintf(stderr, "Error: too many positional arguments for command ");
        print_cmd_stderr(argv, sub_cmds); 
        fprintf(stderr, "expected arg count: %d, actual arg count: %d\n", expected_args, actual_args);
        exit(1);
    }
}

void print_cmd_stderr(char **argv, int sub_cmds) {
    fprintf(stderr, "`modulo");
    for (int i = 0; i < sub_cmds; i++) {
        fprintf(stderr, " %s", argv[i+1]);
    }
    fprintf(stderr, "`\n");
}