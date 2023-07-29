#include <ncurses.h>
#include <stdbool.h>

#include "model.h"
#include "entry_doc.h"
#include "../modulo.h"
#include "screen_model.h"

static bool stage_for_updates(WINDOW *win, WindowModel *win_model);

static void view_update_summary_window(WINDOW *summary_win, Modulo *modulo, WindowModel *summary_model);
static void view_update_doc_window(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc, WindowModel *doc_model);


WINDOW *view_init_doc_window(ScreenModel *screen_model) {
    WindowModel *doc_model = &screen_model->doc_model;
    WINDOW *doc_win = newwin(doc_model->height, doc_model->width, doc_model->top, doc_model->left);
    // scrolling handled explicitly in data model
    scrollok(doc_win, false);
    keypad(doc_win, true);
    return doc_win;
}

WINDOW *view_init_summary_window(ScreenModel *screen_model) {
    WindowModel *summary_model = &screen_model->summary_model;
    WINDOW *summary_win = newwin(summary_model->height, summary_model->width, summary_model->top, summary_model->left);
    scrollok(summary_win, false);
    return summary_win;
}

void view_update(
    WINDOW *doc_win, 
    WINDOW *summary_win, 
    Modulo *modulo, 
    ScreenModel *screen_model,
    EntryDoc *entry_doc
) {
    view_update_summary_window(summary_win, modulo, &screen_model->summary_model);
    view_update_doc_window(doc_win, modulo, entry_doc, &screen_model->doc_model);
}

void view_update_summary_window(WINDOW *summary_win, Modulo *modulo, WindowModel *summary_model) {
    bool update_required = stage_for_updates(summary_win, summary_model);
    if (!update_required) {
        return;
    }
    // update content
    box(summary_win, 0, 0);
    mvwprintw(summary_win, 2, 2, "summary");
    wnoutrefresh(summary_win);
}

void view_update_doc_window(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc, WindowModel *doc_model) {
    static int count = 0;
    bool update_required = stage_for_updates(doc_win, doc_model);
    if (!update_required) {
        return;
    }
    // update content
    box(doc_win, 0, 0);
    count++;
    int h, w;
    getmaxyx(doc_win, h, w);
    int sh, sw;
    getmaxyx(stdscr, sh, sw);
    mvwprintw(doc_win, 2, 2, "line_count: %d", count);
    mvwprintw(doc_win, 3, 2, "win_height: %d win_width: %d", h, w);
    mvwprintw(doc_win, 4, 2, "mod_height: %d mod_width: %d", doc_model->height, doc_model->width);
    mvwprintw(doc_win, 5, 2, "scr_height: %d scr_width: %d", sh, sw);
    wmove(doc_win, 0, 0);
    wnoutrefresh(doc_win);
}

void view_render() {
    doupdate();
}

bool stage_for_updates(WINDOW *win, WindowModel *win_model) {
    int win_h, win_w;
    getmaxyx(win, win_h, win_w);
    if (win_model->height != win_h || win_model->width != win_w) {
        // resize event occurred
        wclear(win);
        wresize(win, win_model->height, win_model->width);
        return true;
    }
    if (win_model->content_update) {
        // content update event occurred
        wclear(win);
        wresize(win, win_model->height, win_model->width);
        return true;
    }
    return false;
}
