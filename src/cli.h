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

Selection cli_prompt_preference_selection();

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