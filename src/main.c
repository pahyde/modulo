#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "command_router.h"

int main(int argc, char **argv) {
    route_command(argc, argv);
    return 0;
}