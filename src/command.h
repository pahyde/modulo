#ifndef COMMAND_H
#define COMMAND_H

#define WAKEUP_BOUNDARY_EARLIEST "earliest"
#define WAKEUP_BOUNDARY_LATEST "latest"

typedef enum {
    DONE,
    PREFERENCE_USERNAME,
    PREFERENCE_WAKEUP_EARLIEST,
    PREFERENCE_WAKEUP_LATEST,
    PREFERENCE_ENTRY_DELIMITER
} Selection;

void command_root();

void command_set_preferences();
void command_set_username(char *username);
void command_set_wakeup_earliest(char *wakeup);
void command_set_wakeup_latest(char *wakeup);
void command_set_entry_delimiter(char *entry_delimiter);

void command_get_preferences();
void command_get_username();
void command_get_wakeup_earliest();
void command_get_wakeup_latest();
void command_get_entry_delimiter();

void command_tomorrow();
void command_today();
void command_peek();
void command_wakeup();
void command_remove(char *entry_number);

void command_history(char *item_number);
void command_history_status();


#endif