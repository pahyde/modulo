#include <ncurses.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"
#include "entry_doc.h"
#include "../modulo.h"
#include "screen_model.h"
#include "view.h"

static bool stage_doc_for_updates(WINDOW *win, DocModel *win_model);
static bool stage_summary_for_updates(WINDOW *win, SummaryModel *win_model);
static bool stage_for_updates(WINDOW *win, int pos_y, int pos_x, int height, int width, bool size_update, bool content_update);

static void view_update_summary_window(WINDOW *summary_win, Modulo *modulo, SummaryModel *summary_model);
static void view_update_doc_window(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc, DocModel *doc_model);

static int min(int a, int b);

static void printf_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *fmt, ...);
static void print_dim(WINDOW *win, SubWindow *sub_win);
static void print_entry_doc_content(WINDOW *doc_win, SubWindow *entry_content, EntryDoc *entry_doc);
static void cpy_line_slice(char *buffer, size_t start_j, size_t end_j, Line *line);


WINDOW *view_init_doc_window(ScreenModel *screen_model) {
    DocModel *doc_model = &screen_model->doc_model;
    WINDOW *doc_win = newwin(doc_model->height, doc_model->width, doc_model->pos_y, doc_model->pos_x);
    // scrolling handled explicitly in data model
    scrollok(doc_win, false);
    keypad(doc_win, true);
    return doc_win;
}

WINDOW *view_init_summary_window(ScreenModel *screen_model) {
    SummaryModel *summary_model = &screen_model->summary_model;
    WINDOW *summary_win = newwin(summary_model->height, summary_model->width, summary_model->pos_y, summary_model->pos_x);
    scrollok(summary_win, false);
    return summary_win;
}

void view_update(
    WINDOW *doc_win, 
    WINDOW *summary_win, 
    Modulo *modulo, 
    ScreenModel *screen_model,
    EntryDoc *entry_doc) {
    view_update_summary_window(summary_win, modulo, &screen_model->summary_model);
    view_update_doc_window(doc_win, modulo, entry_doc, &screen_model->doc_model);
}

void view_update_summary_window(WINDOW *summary_win, Modulo *modulo, SummaryModel *summary_model) {
    bool update_required = stage_summary_for_updates(summary_win, summary_model);
    if (!update_required) {
        return;
    }
    SubWindow *logo = &summary_model->logo;
    SubWindow *entry_list_summary = &summary_model->entry_list_summary;
    // update content
    box(summary_win, 0, 0);
    print_dim(summary_win, logo);
    print_dim(summary_win, entry_list_summary);
    printf_win(summary_win, entry_list_summary, 1, 0, "height: %d, width: %d", summary_model->height, summary_model->width);
}

void view_update_doc_window(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc, DocModel *doc_model) {
    bool update_required = stage_doc_for_updates(doc_win, doc_model);
    if (!update_required) {
        return;
    }
    SubWindow *header = &doc_model->header;
    SubWindow *entry_content = &doc_model->entry_content;
    // update content
    box(doc_win, 0, 0);
    print_entry_doc_content(doc_win, &doc_model->entry_content, entry_doc);
}

void print_dim(WINDOW *win, SubWindow *sub_win) {
    printf_win(win, sub_win, 0, 0, "height: %d, width: %d", sub_win->height, sub_win->width);
}

void view_render(WINDOW *doc_win, WINDOW *summary_win, EntryDoc *entry_doc) {
    //refresh();
    wnoutrefresh(summary_win);
    wnoutrefresh(doc_win);
    doupdate();
}

bool stage_doc_for_updates(WINDOW *doc_win, DocModel *doc_model) {
    stage_for_updates(
        doc_win, 
        doc_model->pos_y,
        doc_model->pos_x,
        doc_model->height,
        doc_model->width,
        doc_model->size_update,
        doc_model->content_update
    );
}

bool stage_summary_for_updates(WINDOW *summary_win, SummaryModel *summary_model) {
    stage_for_updates(
        summary_win, 
        summary_model->pos_y,
        summary_model->pos_x,
        summary_model->height,
        summary_model->width,
        summary_model->size_update,
        summary_model->content_update
    );
}

bool stage_for_updates(WINDOW *win, int pos_y, int pos_x, int height, int width, bool size_update, bool content_update) {
    if (size_update) {
        // resize event occurred
        wclear(win);
        wnoutrefresh(win);
        wresize(win, height, width);
        mvwin(win, pos_y, pos_x);
        return true;
    }
    if (content_update) {
        // content update event occurred
        werase(win);
        return true;
    }
    return false;
}

// TODO: max width and height
void print_entry_doc_content(WINDOW *doc_win, SubWindow *entry_content, EntryDoc *entry_doc) {
    static char buffer[DOC_LINE_BUF_SIZE];

    int height = entry_content->height;
    int width = entry_content->width;
    Index *scroll = &entry_doc->scroll;

    // get start and end (exclusive) line index
    size_t start_i = scroll->i;
    size_t end_i = min(scroll->i + height, entry_doc->line_count);
    for (size_t i = start_i; i < end_i; i++) {
        // get start and end (exclusive) char index
        Line *line = entry_doc_get_line(entry_doc, i);
        size_t start_j = scroll->j;
        size_t end_j = min(scroll->j + width, line->length);
        // get visible slice and print to virt screen
        cpy_line_slice(buffer, start_j, end_j, line->chars);
        // TODO > (2)
        printf_win(doc_win, entry_content, i, 0, buffer);
    }
}

void cpy_line_slice(char *buffer, size_t start_j, size_t end_j, Line *line) {
    size_t length = end_j - start_j;
    memcpy(buffer, line, length * sizeof(char));
    buffer[length] = '\0';
}

void printf_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int y = sub_win->pos_y + sub_win->top;
    int x = sub_win->pos_x + sub_win->left;
    wmove(win, y + offset_y, x + offset_x);
    vwprintw(win, fmt, args);
}

int min(int a, int b) {
    return a < b ? a : b;
}