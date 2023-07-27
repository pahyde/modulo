#include <ncurses.h>

#include "../modulo.h"
#include "../filesystem.h"
#include "entry_doc.h"


/*
Lesson: input -> Document -> check_scroll(Document, Screen) -> WINDOW -> Terminal
             \-> Screen /

Another example of chronological separation 
*/

static void screen_init();
static WINDOW *summary_window_init();
static WINDOW *document_window_init();

static void summary_window_update();
static void document_window_update();

static void summary_window_render();
static void document_window_render();


/*

model:
1) modulo: tomorrow, wakeup[earliest|latest] 
2) EntryDoc: lines, cursor
3) Editor Context
    - width, height
    - offseth, offsetv

controller
char c = wgetch(editor_win)
switch (c)
cases:
    0. ascii input
    1. SIGINT
    2. EOF
    3. backspace (delete?)
    4. enter (crlf)
    5. 

view
window_offseth +
window_offsetv +
editor->lines +
editor->cursor ===> editor view


    int h, w;
    getmaxyx(stdscr, h, w);
    WindowFrame doc_frame = init_doc_frame(h, w)
    WindowFrame summary_frame = init_summary_frame(h, w)

    WINDOW *doc_win = init_doc_window()
    WINDOW *summary_win = init_summary_window();

    Stages:
        > display UI from model
            0. refresh(doc_win), refresh(summary_win)
        > wait for user input
            1. user input -> (modulo, entry_doc, doc_frame, summary_frame)
        > update model to reflect input
            2. check_scroll(entry_doc, screen_ctx)
            3. update_doc_window(doc_window, doc_frame, entry_doc)
            4. update_summary_window(summary_window, summary_frame, modulo)


    Document {
        capacity
        line_count
        char **lines
        cursor { y x }
        scroll { y x }
    }

    Window {
        height
        width
        offset_y
        offset_x
        is_hidden
    }


    struct EditorEvent {
        type: ENTRY_DELIM, EXIT, RESIZE, KEY_PRESS
        input:  
    } EditorEvent

    
    if (!wm_summary_hidden()) {
        summary_win_update()
    }


    }
    

    ScreenModel {
        WindowModel doc_model
        WindowModel summary_model
    }


    void handle_key_press(.., input) {
        switch (input) {
            handle_*(entry_doc, [screen])
            case: arrow key -> handle_arrow_key()
            case: backsapce -> handle_backspace()
            case: delete -> handle_delete()
            case: delim -> handle_delim()
            case: crlf -> handle_enter()
            case: EOF, SIGINT -> handle_terminate()
            default:  -> handle_char_input()
        }
    }


*/
start_entry_editory(Modulo *modulo) {
    ScreenModel screen_model = create_screen_model();
    EntryDoc *entry_doc = create_entry_doc();

    WINDOW *doc_win = view_init_doc_window(screen_model);
    WINDOW *summary_win = view_init_summary_window(screen_model);
    
    while (true) { 
        // update view from model
        view_update(doc_win, summary_win, modulo, entry_doc, screen_model);
        // Render view to terminal
        view_render(doc_win, summary_win);

        // view_update(modulo, entry_doc, wm, doc_win, summary_win)
        if (!wm_summary_hidden(screen)) {
            summary_window_update(summary_win, modulo, entry_doc, screen);
        }
        doc_window_update(doc_win, modulo, entry_doc, screen);

        // view_render(doc_win, summary_win)
        
        // get user input
        EditorEvent event = get_user_event();
        if (event->type == EXIT) {
            entry_editor_save_entry(modulo, c, entry);
            break;
        }
        switch (event->type) {
            case ENTRY_DELIM:
                handle_entry_delim(modulo, c, entry_doc);
                break;
            case RESIZE:
                handle_resize(modulo, wm);
                break;
            case KEY_PRESS:
                handle_key_press(modulo, entry_doc, event->input);
                break;
            default:
                // shouldn't happen
        }
        check_scroll(wm, entry_doc);
        // view_stage()
        summary_window_stage()
        doc_window_stage()
    }
}

void entry_editor_start(modulo) {
    screen_init();


    int h, w;
    getmaxyx(stdscr, h, w);

    int panel_width = 28;
    WINDOW *win1 = newwin(LINES, panel_width, 0, 0);
    WINDOW *win2 = newwin(LINES, w-panel_width, 0, panel_width);
    keypad(win1, true);
    keypad(win2, true);

    //WINDOW *win1 = summary_window_init();
    //WINDOW *win2 = document_window_init();

    int x = 0;
    int y = 0;
    bool is_resize = false;
    while (true) {
        wclear(win1);
        wclear(win2);
        if (is_resize) {
            getmaxyx(stdscr, h, w);
            wresize(win2, h, w-panel_width);
            wresize(win1, h, panel_width);
        }
        box(win1, 0, 0);

        box(win2, 0, 0);
        wmove(win2, 1,1);
        wprintw(win2, "This is a test");

        if (x < 0) x = 0;
        if (x >= w-panel_width) x = w-panel_width-1;
        if (y < 0) y = 0;
        if (y >= h) y = h-1;
        wmove(win2, y, x);
        wnoutrefresh(win1);
        wnoutrefresh(win2);
        doupdate();
        int c = wgetch(win2);
        switch (c) {
            case KEY_UP:
                y -= 1;
                break;
            case KEY_DOWN:
                y += 1;
                break;
            case KEY_RIGHT:
                x += 1;
                break;
            case KEY_LEFT:
                x -= 1;
                break;
            case KEY_RESIZE:
                is_resize = true;
                break;
            default:
                break;
        }
    }
}

void screen_init() {
    initscr();
    cbreak();
    noecho();
}



/*

entry_editor_get_entry() {
        // 
        entry_editor_update_view(modulo, entry_doc, editor_win);
        EntryDoc entry_doc = entry_editor_write_entry(modulo, editor_win);
        char *entry = entry_doc_to_string(entry_doc);
        modulo_push_tomorrow(modulo, entry);
        wclear(editor_win);
        if (entry_doc.is_last_entry) {
            break;
        }
    initscr();
    cbreak();
    scrollok(stdscr, true);
    keypad(stdscr, true);
    noecho();
    printw("Hello ncurses!");
    refresh();
    getch();
    int ch = mvwinch(stdscr, 0, 0);  
    char c = ch & A_CHARTEXT;  
    attr_t attr = ch & A_ATTRIBUTES;
    move(1,0);
    printw("%c", c);
    refresh();
    getch();
    endwin(); 
}
*/