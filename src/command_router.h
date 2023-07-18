#ifndef COMMAND_ROUTER_H
#define COMMAND_ROUTER_H

/* modulo command keywords */
#define COMMAND_INIT "init"
#define COMMAND_SET "set"
#define COMMAND_GET "get"
#define COMMAND_PREFERENCES "preferences"
#define COMMAND_USERNAME "username"
#define COMMAND_WAKEUP_EARLIEST "wakeup_earliest"
#define COMMAND_WAKEUP_LATEST "wakeup_earliest"
#define COMMAND_ENTRY_DELIMITER "entry_delimiter"

#define COMMAND_TOMORROW "tomorrow"
#define COMMAND_PEEK "peek"
#define COMMAND_WAKEUP "wakeup"
#define COMMAND_TODAY "today"
#define COMMAND_REMOVE "remove"

#define COMMAND_HISTORY "history"


void command_router(int argc, char **argv);

#endif