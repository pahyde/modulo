#ifndef ACTIONS_H
#define ACTIONS_H

#include "main.h"

Modulo *create_default_modulo(char *username);

void sync(Modulo *modulo);
void sync_data_with_timestamp(Modulo *modulo, time_t now);

#endif