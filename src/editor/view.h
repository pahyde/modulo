#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>

#include "model.h"
#include "entry_doc.h"
#include "../modulo.h"
#include "screen_model.h"

#define DOC_LINE_BUF_SIZE 512

#define ENTRY_PREVIEW_LENGTH 14

#define LOGO_OFFSET_X 2

WINDOW *view_init_doc_window(ScreenModel *screen_model);
WINDOW *view_init_summary_window(ScreenModel *screen_model);

void view_update(WINDOW *doc_win, WINDOW *summary_win, Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc);
void view_render(WINDOW *doc_win, WINDOW *summary_win, EntryDoc *entry_doc);

#endif