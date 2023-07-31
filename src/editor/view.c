#include <ncurses.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
static int max(int a, int b);

static void print_entry_doc_header(WINDOW *doc_win, SubWindow *header, Modulo *modulo, EntryDoc *entry_doc);
static void print_entry_doc_content(WINDOW *doc_win, SubWindow *entry_content, EntryDoc *entry_doc);
static void print_modulo_logo(WINDOW *summary_win, SubWindow *logo);
static void print_entry_summary(WINDOW *summary_win, SubWindow *entry_list_summary, Modulo *modulo);
static char *get_entry_preview(const char *entry);
static void printf_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *fmt, ...);
static void print_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *line);
static void print_dim(WINDOW *win, SubWindow *sub_win);
static void hline_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, int width);

static void doc_move_cursor(WINDOW *doc_win, SubWindow *entry_content, EntryDoc *entry_doc);
static void cpy_line_slice(char *buffer, size_t start_j, size_t end_j, char *line);


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
    print_modulo_logo(summary_win, logo);
    print_entry_summary(summary_win, entry_list_summary, modulo);
}

void print_modulo_logo(WINDOW *summary_win, SubWindow *logo) {
    print_win(summary_win, logo, 0, LOGO_OFFSET_X, " ___   /\\      ");
    print_win(summary_win, logo, 1, LOGO_OFFSET_X, "|\\__\\ / /\\     ");
    print_win(summary_win, logo, 2, LOGO_OFFSET_X, "\\|__|/ / /     ");
    print_win(summary_win, logo, 3, LOGO_OFFSET_X, "    / / /___   ");
    print_win(summary_win, logo, 4, LOGO_OFFSET_X, "   / / /|\\__\\  ");
    print_win(summary_win, logo, 5, LOGO_OFFSET_X, "  |\\/ / \\|__|  ");
    print_win(summary_win, logo, 6, LOGO_OFFSET_X, "   \\|/         ");
}

void print_entry_summary(WINDOW *summary_win, SubWindow *entry_list_summary, Modulo *modulo) {
    print_win(summary_win, entry_list_summary, 0, 0, "Entries:");

    int list_offset_y = 2;
    int list_height = entry_list_summary->height - list_offset_y;

    // choose start and end (exclusive) index 
    // to accomodate for list of completed entries
    // and an additional (IN PROGRESS) entry
    size_t end_idx = modulo->tomorrow.size + 1;
    size_t start_idx = max(0, end_idx - list_height);
    print_entry_previews(summary_win, entry_list_summary, modulo, list_offset_y, start_idx, end_idx);
}

void print_entry_previews(
    WINDOW *summary_win, 
    SubWindow *entry_list_summary, 
    Modulo *modulo, 
    int list_offset_y, 
    int start_idx, 
    int end_idx
) {
    EntryList *tomorrow = &modulo->tomorrow;
    // print complete entry previews
    int y_offset = list_offset_y;
    for (int i = start_idx; i < end_idx-1; i++) {
        char *entry_preview = get_entry_preview(tomorrow->entries[i]);
        printf_win(summary_win, entry_list_summary, y_offset, 0, "%d. %s", i+1, entry_preview);
        y_offset++;
    }
    // print placeholder for current entry
    printf_win(summary_win, entry_list_summary, y_offset, 0, "%d. (IN PROGRESS)", end_idx);
}

char *get_entry_preview(const char *entry) {
    static char preview[ENTRY_PREVIEW_LENGTH];
    char *c;
    // skip leading spaces
    for (c = entry; *c != '\0'; c++) {
        if (!isspace(*c)) {
            break;
        }
    }
    size_t ellipsis_start = ENTRY_PREVIEW_LENGTH - 4;
    strcpy(&preview[ellipsis_start], "...");

    size_t length = strlen(c);
    while (ellipsis_start > length) {
        preview[--ellipsis_start] = '.';
    }
    for (int i = ellipsis_start-1; i >= 0; i--) {
        if (!isspace(c[i])) {
            break;
        }
        preview[i] = '.';
        ellipsis_start = i;
    }
    for (size_t i = 0; i < ellipsis_start; i++) {
        preview[i] = isspace(c[i]) ? ' ' : c[i];
    }
    return preview;
}

void view_update_doc_window(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc, DocModel *doc_model) {
    bool update_required = stage_doc_for_updates(doc_win, doc_model);
    if (!update_required) {
        return;
    }
    SubWindow *header = &doc_model->header;
    SubWindow *entry_content = &doc_model->entry_content;
    Index cursor = entry_doc_get_effective_cursor(entry_doc);
    // update content
    box(doc_win, 0, 0);
    //printf_win(doc_win, header, 0, 0, "cursor i: %d, j: %d", cursor.i, cursor.j);
    //printf_win(doc_win, header, 1, 0, "scroll i: %d, j: %d", entry_doc->scroll.i, entry_doc->scroll.j);
    print_entry_doc_header(doc_win, &doc_model->header, modulo, entry_doc);
    print_entry_doc_content(doc_win, &doc_model->entry_content, entry_doc);
    doc_move_cursor(doc_win, entry_content, entry_doc);
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
    return stage_for_updates(
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
    return stage_for_updates(
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

void print_entry_doc_header(WINDOW *doc_win, SubWindow *header, Modulo *modulo, EntryDoc *entry_doc) {
    size_t line_count = entry_doc->header.line_count;
    // This time no flex. Checkout screen_model.c:123 for the flex.
    for (size_t i = 0; i < line_count; i++) {
        print_win(doc_win, header, i, 0, entry_doc->header.lines[i]);
    }
    hline_win(doc_win, header, line_count, 0, header->width);
    printf_win(doc_win, header, line_count+2, 0, "Entry %d.", modulo->tomorrow.size+1);
}

void hline_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, int width) {
    int y = sub_win->pos_y + sub_win->top;
    int x = sub_win->pos_x + sub_win->left;
    wmove(win, y + offset_y, x + offset_x);
    whline(win, ACS_HLINE, width);
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
        // TODO constrain(value, min, max)
        size_t end_j = max(scroll->j, min(scroll->j + width, line->length));
        // get visible slice and print to virt screen
        cpy_line_slice(buffer, start_j, end_j, line->chars);
        // TODO > (2)
        print_win(doc_win, entry_content, i-start_i, 0, buffer);
    }
}

void doc_move_cursor(WINDOW *doc_win, SubWindow *entry_content, EntryDoc *entry_doc) {
    Index cursor = entry_doc_get_effective_cursor(entry_doc);
    Index scroll = entry_doc->scroll;
    int i = entry_content->pos_y + entry_content->top;
    int j = entry_content->pos_x + entry_content->left;
    int i_offset = cursor.i - scroll.i;
    int j_offset = cursor.j - scroll.j;
    wmove(doc_win, i + i_offset, j + j_offset);
}

void cpy_line_slice(char *buffer, size_t start_j, size_t end_j, char *line) {
    size_t length = end_j - start_j;
    memcpy(buffer, (line + start_j), length * sizeof(char));
    buffer[length] = '\0';
}

void printf_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int y = sub_win->pos_y + sub_win->top;
    int x = sub_win->pos_x + sub_win->left;
    wmove(win, y + offset_y, x + offset_x);
    vw_printw(win, fmt, args);
}

void print_win(WINDOW *win, SubWindow *sub_win, int offset_y, int offset_x, const char *line) {
    int y = sub_win->pos_y + sub_win->top;
    int x = sub_win->pos_x + sub_win->left;
    wmove(win, y + offset_y, x + offset_x);
    waddstr(win, line);
}

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }