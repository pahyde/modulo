#include <stdio.h>
#include <ncurses.h>
#include <string.h>

#include "../modulo.h"
#include "../filesystem.h"
#include "entry_doc.h"
#include "screen_model.h"

static void remove_exit_delim(Modulo *modulo, EntryDoc *entry_doc);
static void remove_entry_delim(Modulo *modulo, EntryDoc *entry_doc);
static void submit_entry(Modulo *modulo, EntryDoc *entry_doc);
static void log_doc_update(ScreenModel *screen_model);
static void log_summary_update(ScreenModel *screen_model);
static void save_modulo_or_exit(Modulo *modulo, OSContext *c);

static bool is_empty(EntryDoc *entry_doc);
static char *entry_doc_to_string(EntryDoc *entry_doc);
static int max(int a, int b);

void model_reset(ScreenModel *screen_model) {
    DocModel *doc_model = &screen_model->doc_model;
    SummaryModel *summary_model = &screen_model->summary_model;
    doc_model->content_update = false;
    doc_model->size_update = false;
    summary_model->content_update = false;
    summary_model->size_update = false;
}

void model_handle_exit(Modulo *modulo, OSContext *c, EntryDoc *entry_doc) {
    remove_exit_delim(modulo, entry_doc);
    if (is_empty(entry_doc)) {
        return;
    }
    submit_entry(modulo, entry_doc);
    save_modulo_or_exit(modulo, c);
}

void model_handle_entry_submit(Modulo *modulo, OSContext *c, ScreenModel *screen_model, EntryDoc *entry_doc) {
    remove_entry_delim(modulo, entry_doc);
    submit_entry(modulo, entry_doc);
    save_modulo_or_exit(modulo, c);
    entry_doc_clear(entry_doc);
    log_doc_update(screen_model);
    log_summary_update(screen_model);
}

void model_handle_resize(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc) {
    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);
    screen_model_resize(screen_model, entry_doc, screen_h, screen_w);
}

void model_handle_backspace(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc) {
    entry_doc_backspace(entry_doc);
    log_doc_update(screen_model);
}

void model_handle_enter(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc) {
    entry_doc_enter(entry_doc);
    log_doc_update(screen_model);
}

void model_handle_cursor_move(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, int dir) {
    switch (dir) {
        case KEY_UP:
            entry_doc_cursor_up(entry_doc, true);
            break;
        case KEY_DOWN:
            entry_doc_cursor_up(entry_doc, true);
            break;
        case KEY_LEFT:
            entry_doc_cursor_up(entry_doc, true);
            break;
        case KEY_RIGHT:
            entry_doc_cursor_up(entry_doc, true);
            break;
        default:
            // shouldn't happen
            fprintf(stderr, "Unrecognized cursor move event\n");
            exit(EXIT_FAILURE);
    }
    log_doc_update(screen_model);
}

void model_handle_char_input(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, char input) {
    entry_doc_insert_char(entry_doc, input);
    log_doc_update(screen_model);
}

void model_handle_no_event(ScreenModel *screen_model) { return; }

void model_check_scroll(ScreenModel *screen_model, EntryDoc *entry_doc) {
    Index cursor = entry_doc_get_cursor(entry_doc);
    Index *scroll = &entry_doc->scroll;
    SubWindow *entry_doc_content = &screen_model->doc_model.entry_content;
    int content_height = entry_doc_content->height;
    int content_width = entry_doc_content->width;
    scroll->i = max(scroll->i, cursor.i - (content_height-1));
    scroll->j = max(scroll->j, cursor.j - (content_width-1));
}

int max(int a, int b) {
    return a > b ? a : b;
}
    
void save_modulo_or_exit(Modulo *modulo, OSContext *c) {
    if (save_modulo(modulo, c) == -1) {
        fprintf(stderr, "An error occurred saving the last entry!\n");
        exit(EXIT_FAILURE);
    }
}

void submit_entry(Modulo *modulo, EntryDoc *entry_doc) {
    char *entry = entry_doc_to_string(entry_doc);
    modulo_push_tomorrow(modulo, entry);
}

void log_doc_update(ScreenModel *screen_model) {
    screen_model->doc_model.content_update = true;
}

void log_summary_update(ScreenModel *screen_model) {
    screen_model->summary_model.content_update = true;
}

void remove_exit_delim(Modulo *modulo, EntryDoc *entry_doc) {
    size_t entry_delim_length = strlen(modulo->entry_delimiter);
    for (size_t i = 0; i < 2*entry_delim_length; i++) {
        entry_doc_backspace(entry_doc);
    }
}

void remove_entry_delim(Modulo *modulo, EntryDoc *entry_doc) {
    size_t entry_delim_length = strlen(modulo->entry_delimiter);
    for (size_t i = 0; i < entry_delim_length; i++) {
        entry_doc_backspace(entry_doc);
    }
}

char *entry_doc_to_string(EntryDoc *entry_doc) {
    size_t line_count = entry_doc->line_count;
    size_t char_count = 0;
    for (size_t i = 0; i < line_count; i++) {
        char_count += entry_doc_get_line(entry_doc, i)->length;
    }
    // strlen = char_count + line_count (newlines) + 1 (null terminator)
    char *entry_string = malloc((line_count + char_count + 1) * sizeof(char));
    size_t start = 0;
    for (size_t i = 0; i < line_count; i++) {
        Line *line = entry_doc_get_line(entry_doc, i);
        size_t length = line->length;
        memcpy(&entry_string[start], line, length);
        entry_string[start+length] = '\n';
        start = length+1;
    }
    entry_string[start] = '\0';
    return entry_string;
}

bool is_empty(EntryDoc *entry_doc) {
    return entry_doc->line_count == 1 && entry_doc_get_line(entry_doc, 0)->length == 0;
}