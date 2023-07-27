#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>

#include "model.h"
#include "entry_doc.h"
#include "../modulo.h"
#include "screen_model.h"

void view_update(
    WINDOW *doc_win, 
    WINDOW *summary_win, 
    Modulo *modulo, 
    EntryDoc *entry_doc, 
    ScreenModel *screen_model);

void view_render(WINDOW *doc_win, WINDOW *summary_win);

#endif