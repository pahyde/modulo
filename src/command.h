#ifndef COMMAND_H
#define COMMAND_H

/* modulus command keywords */
#define INIT "init"

#define SET "set"
#define GET "get"
#define PREFERENCES "preferences"
#define USERNAME "username"
#define WAKEUP "wakeup"
#define ENTRY_DELIMITER "entry_delimiter"

#define TOMORROW "tomorrow"
#define TODAY "today"
#define PEEK "peek"

#define REMOVE "remove"

void command_root();

void command_set_preferences();
void command_set_username(char *username);
void command_set_wakeup(char *wakeup);
void command_set_entry_delimiter(char *entry_delimiter);

void command_get_preferences();
void command_get_username();
void command_get_wakeup();
void command_get_entry_delimiter();

void command_tomorrow();
void command_today();
void command_peek();

void command_remove(char *entry_number);

#endif