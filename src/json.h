#ifndef JSON_H
#define JSON_H

#include <cjson/cJSON.h>

#include "main.h"

Modulo *json_to_modulo(cJSON *json);
cJSON *modulo_to_json(Modulo *modulo);

#endif