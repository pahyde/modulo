#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

#include "modulo.h"
#include "entry_list.h"

void cli_print_init_message(char *username);

void cli_prompt_day_ptr(int wakeup_earliest, int wakeup_latest);

void cli_prompt_username(Modulo *modulo, bool show_prev);
void cli_prompt_wakeup_earliest(Modulo *modulo, bool show_prev);
void cli_prompt_wakeup_latest(Modulo *modulo, bool show_prev);
void cli_prompt_entry_delimiter(Modulo *modulo, bool show_prev);

int cli_set_username(Modulo *modulo, char *username, bool show_prev);
int cli_set_wakeup_earliest(Modulo *modulo, char *wakeup, bool show_prev);
int cli_set_wakeup_latest(Modulo *modulo, char *wakeup, bool show_prev);
int cli_set_entry_delimiter(Modulo *modulo, char *entry_delimiter, bool show_prev);

#endif