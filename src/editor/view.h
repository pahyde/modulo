#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>

#include "model.h"
#include "entry_doc.h"
#include "../modulo.h"
#include "screen_model.h"

WINDOW *view_init_doc_window(ScreenModel *screen_model);
WINDOW *view_init_summary_window(ScreenModel *screen_model);

void view_update(WINDOW *doc_win, WINDOW *summary_win, Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc);
void view_render();

#endif