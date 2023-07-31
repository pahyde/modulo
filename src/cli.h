#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

#include "modulo.h"
#include "entry_list.h"
#include "command.h"

typedef enum FsmState {
    START,
    RAW,
    RAW_ESCAPE,
    QUOTED,
    QUOTED_ESCAPE,
    SPACE,
    INVALID
} FsmState;

#define CLI_DONE "done"
#define MAX_INPUT_LENGTH 63

void cli_print_init_hello(char *username);
void cli_print_init_goodbye(Modulo *modulo);
void cli_print_preferences(Modulo *modulo);
void cli_print_wakeup_success(Modulo *modulo);
void cli_print_wakeup_failure(Modulo *modulo);
void cli_print_time_status(Modulo *modulo);
void cli_print_entry_lists_status(Modulo *modulo);
void cli_print_today_entries(Modulo *modulo);

void cli_print_history_status(HistoryQueue *history);
void cli_print_history_item(HistoryQueue *history, int entry_list_index);

Selection cli_prompt_preference_selection();

bool cli_prompt_yes_or_no();

void cli_prompt_day_ptr(Modulo *modulo, time_t recent_wakeup_earliest, time_t recent_wakeup_latest);
void cli_prompt_username(Modulo *modulo, bool show_prev);
void cli_prompt_wakeup_earliest(Modulo *modulo, bool show_prev);
void cli_prompt_wakeup_latest(Modulo *modulo, bool show_prev);
void cli_prompt_entry_delimiter(Modulo *modulo, bool show_prev);

int cli_set_username(Modulo *modulo, char *username, bool show_prev);
int cli_set_wakeup_earliest(Modulo *modulo, char *wakeup, bool show_prev);
int cli_set_wakeup_latest(Modulo *modulo, char *wakeup, bool show_prev);
int cli_set_entry_delimiter(Modulo *modulo, char *entry_delimiter, bool show_prev);

#endif