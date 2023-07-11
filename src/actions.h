#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdbool.h>

#include "main.h"

Modulo *create_default_modulo(char *username);
void free_modulo(Modulo *modulo);

void update_username(Modulo *modulo, char *username);

bool out_of_sync(Modulo *modulo);
void sync(Modulo *modulo);
void sync_data_with_timestamp(Modulo *modulo, time_t now);

#endif