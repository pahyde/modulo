#include <stdio.h>
#include <ncurses.h>

#include "../modulo.h"
#include "../filesystem.h"
#include "entry_doc.h"
#include "screen_model.h"

static void submit_entry(Modulo *modulo, EntryDoc *entry_doc);
static void log_updates(ScreenModel *screen_model, bool doc_updated, bool summary_updated);

void model_handle_exit(Modulo *modulo, OSContext *c, EntryDoc *entry_doc) {
    remove_exit_delim(entry_doc);
    if (is_empty(entry_doc)) {
        return;
    }
    submit_entry(modulo, entry_doc);
    save_modulo_or_exit(modulo, c);
}

void model_handle_entry_submit(Modulo *modulo, OSContext *c, ScreenModel *screen_model, EntryDoc *entry_doc) {
    remove_entry_delim(entry_doc);
    submit_entry(modulo, entry_doc);
    save_modulo_or_exit(modulo, c);
    entry_doc_clear(entry_doc);
    log_updates(screen_model, true, true);
}

void model_handle_resize(Modulo *modulo, ScreenModel *screen_model) {
    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);
    screen_model_resize(screen_model, screen_h, screen_w);
    log_updates(screen_model, false, false);
}

void model_handle_backspace(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc) {
    entry_doc_backspace(entry_doc);
    log_updates(screen_model, true, false);
}

void model_handle_enter(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc) {
    entry_doc_enter(entry_doc);
    log_updates(screen_model, true, false);
}

void model_handle_cursor_move(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, char dir) {
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
    log_updates(screen_model, true, false);
}

void model_handle_char_input(Modulo *modulo, ScreenModel *screen_model, EntryDoc *entry_doc, char input) {
    entry_doc_insert_char(entry_doc, input);
    log_updates(screen_model, true, false);
}

void model_handle_no_event(ScreenModel *screen_model) {
    log_updates(screen_model, false, false);
}

void model_check_scroll(ScreenModel *screen_model, EntryDoc *entry_doc) {
    /*
    entry_doc_header - height - width
    entry_doc_content - height - width

    wprint_header(2,3)
    wprint_content(0,1)
    */
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

void log_updates(ScreenModel *screen_model, bool doc_updated, bool summary_updated) {
    screen_model->doc_model.content_update = doc_updated;
    screen_model->summary_model.content_update = summary_updated;
}