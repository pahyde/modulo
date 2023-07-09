#ifndef COMMAND_H
#define COMMAND_H

/* modulus command keywords */
#define INIT "init"

#define SET "set"
#define USERNAME "username"
#define WAKEUP "wakeup"

#define TOMORROW "tomorrow"
#define TODAY "today"
#define PEEK "peek"

#define REMOVE "remove"

void command_init(int argc, char **argv);

void command_set(int argc, char **argv);
void command_set_username(char *name);
void command_set_wakeup(char *name);

void command_tomorrow(int argc, char **argv);
void command_today(int argc, char **argv);
void command_peek(int argc, char **argv);

void command_remove(int argc, char **argv);

#endif