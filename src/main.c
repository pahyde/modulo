#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "command.h"

int main(int argc, char **argv) {

    if (argc == 1) {
        // modulus root command
        // display minimal usage hints
        printf("Modulus is a minimal productivity app designed for continuity!\n");
        printf("It allows you to offload end-of-day thoughts, motivations, and goals onto tomorrows to-do list.\n"); 
        printf("\n"); 
        printf("Run `modulus init` to setup your user preferences.\n");
        printf("Then run `modulus tomorrow` to start recording your thoughts for tomorrow!\n\n");
        return 0;
    }
    // primary command
    char *cmd = argv[0];
    if (strcmp(cmd, INIT) == 0) {
        command_init(argc, argv);
    } else if (strcmp(cmd, SET) == 0) {
        command_set(argc, argv);
    } else if (strcmp(cmd, TOMORROW) == 0) {
        command_tomorrow(argc, argv);
    } else if (strcmp(cmd, TODAY) == 0) {
        command_today(argc, argv);
    } else if (strcmp(cmd, PEEK) == 0) {
        command_peek(argc, argv);
    } else if (strcmp(cmd, REMOVE) == 0) {
        command_remove(argc, argv);
    } else {
        printf("Bad command: %s", cmd);
        exit(-1);
    }
    /*
    Program flow:
        1. parse arguments
        2. handleErrors
        3. Execute commmand
            - modulus: 
                documentation

            - modulus init:
                set name
                set wakeup
                set eof

            - modulus set name
            - modulus set wakeup
            - modulus set eof

            - modulus tomorrow:
                sdfgdf
                sdfd % 
            - modulus peek
            - modulus today
    */
    return 0;
}