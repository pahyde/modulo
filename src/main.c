#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "command_router.h"
#include "time_utils.h"

int main(int argc, char **argv) {
    command_router(argc, argv);
    return 0;
}