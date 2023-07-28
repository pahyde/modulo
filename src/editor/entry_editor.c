#include <ncurses.h>
#include <ctype.h>
#include <string.h>

#include "../modulo.h"
#include "../filesystem.h"
#include "entry_editor.h"
#include "entry_doc.h"
#include "screen_model.h"
#include "view.h"


/*
Lesson: input -> Document -> check_scroll(Document, Screen) -> WINDOW -> Terminal
             \-> Screen /

Another example of chronological separation 
*/

static void screen_init(int *screen_h, int *screen_w);
static EditorEvent get_user_event(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc);
static void screen_exit(WINDOW *doc_win, WINDOW *summary_win);



start_entry_editory(Modulo *modulo, OSContext *c) {
    int screen_h, screen_w;
    screen_init(&screen_h, &screen_w);

    ScreenModel *screen_model = create_screen_model(screen_h, screen_w);
    EntryDoc *entry_doc = create_entry_doc();

    WINDOW *doc_win = view_init_doc_window(screen_model);
    WINDOW *summary_win = view_init_summary_window(screen_model);
    
    while (true) { 
        // update view from model
        view_update(doc_win, summary_win, modulo, screen_model, entry_doc);
        // Render view to terminal
        view_render();

        // get user input
        EditorEvent event = get_user_event(doc_win, modulo, entry_doc);
        if (event.type == EXIT) {
            model_handle_exit(modulo, c, entry_doc);
            break;
        }
        switch (event.type) {
            case ENTRY_SUBMIT:
                model_handle_entry_submit(modulo, c, screen_model, entry_doc);
                break;
            case RESIZE:
                model_handle_resize(modulo, screen_model);
                break;
            case BACKSPACE:
                model_handle_backspace(modulo, screen_model, entry_doc);
                break;
            case ENTER:
                model_handle_enter(modulo, screen_model, entry_doc);
                break;
            case CURSOR_MOVE:
                model_handle_cursor_move(modulo, screen_model, entry_doc, event.input);
                break;
            case CHAR_INPUT:
                model_handle_char_input(modulo, screen_model, entry_doc, event.input);
                break;
            case NONE:
                model_handle_no_event(screen_model);
            default:
                // shouldn't happen
                fprintf(stderr, "unexpected event type %d\n", event.type);
                exit(EXIT_FAILURE);
        }
        model_check_scroll(screen_model, entry_doc);
    }
    free_screen_model(screen_model);
    free_entry_doc(entry_doc);
    screen_exit(doc_win, summary_win);
}

EditorEvent get_user_event(WINDOW *doc_win, Modulo *modulo, EntryDoc *entry_doc) {
    static EditorEvent event;
    int c = wgetch(doc_win);
    switch (c) {
        case KEY_ENTER:
        case '\n':
        case '\r':
            EventType type = get_enter_event_type(modulo, entry_doc);
            event = (EditorEvent) { .type = type };
            break;
        case KEY_BACKSPACE:
            event = (EditorEvent) { .type = BACKSPACE };
            break;
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            event = (EditorEvent) { .type = CURSOR_MOVE, .input = c };
            break;
        case KEY_RESIZE:
            event = (EditorEvent) { .type = RESIZE };
            break;
    }
    if (is_char_input(c)) {
        event = (EditorEvent) { .type = CHAR_INPUT, .input = c };
    } else {
        event = (EditorEvent) { .type = NONE };
    }
    return event;
}

EventType get_enter_event_type(Modulo *modulo, EntryDoc *entry_doc) {
    Index cursor = entry_doc_get_cursor(entry_doc);
    char *entry_delim = modulo_get_entry_delimiter(modulo);
    Line *line = entry_doc_get_line(entry_doc, cursor.i);
    size_t delim_length = strlen(entry_delim);
    // check for single delim
    if (cursor.j < delim_length) {
        return ENTER;
    }
    if (strncmp(entry_delim, &line->chars[cursor.j-delim_length], delim_length) != 0) {
        return ENTER;
    }
    // check for double delim
    if (cursor.j < 2*delim_length) {
        return ENTRY_SUBMIT;
    }
    if (strncmp(entry_delim, &line->chars[cursor.j-2*delim_length], delim_length) != 0) {
        return ENTRY_SUBMIT;
    }
    return EXIT;
}

bool is_char_input(int c) {
    return isalnum(c) || ispunct(c) || isspace(c);
}

void screen_init(int *screen_h, int *screen_w) {
    initscr();
    cbreak();
    noecho();
    getmaxyx(stdscr, *screen_h, *screen_w);
}

void screen_exit(WINDOW *doc_win, WINDOW *summary_win) {
    delwin(doc_win);
    delwin(summary_win);
    endwin();
}
