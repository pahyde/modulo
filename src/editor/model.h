#ifndef MODEL_H
#define MODEL_H

#include "../modulo.h"
#include "../filesystem.h"
#include "entry_doc.h"
#include "screen_model.h"

void model_reset(ScreenModel *screen_model);

void model_handle_exit(Modulo *modulo, OSContext *c, EntryDoc *entry_doc);
void model_handle_entry_submit(Modulo *modulo, OSContext *c, ScreenModel *screen_model, EntryDoc *entry_doc);
void model_handle_resize(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc);
void model_handle_backspace(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc);
void model_handle_enter(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc);
void model_handle_cursor_move(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, int dir);
void model_handle_char_input(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, char input);
void model_handle_no_event(ScreenModel *screen_model);

void model_check_scroll(ScreenModel *screen_model, EntryDoc *entry_doc);

#endif