#include <ncurses.h>

#include "modulo.h"

static WINDOW *box_win(int h, int w, int y, int x);
static void entry_editor_init();


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

    summary_window_init();
    summary_window_update(modulo, entry_doc);

    editor_window_init();
    editor_window_update(modulo, entry_doc);

    EntryDoc {
        capacity
        line_count
        char **lines
        cursor { y x }
        scroll { y x }
        width 
        height
    }

*/
void entry_editor_start(modulo) {
    initscr();

    int h, w;
    getmaxyx(stdscr, h, w);

    int panel_width = 28;
    mvvline(0, panel_width, ACS_VLINE, 10); 
    refresh();

    WINDOW *win = newwin(LINES, panel_width, 0, 0);
    wmove(win, 0,0);
    wprintw(win,"Date: July 24, 2023");
    wmove(win, 1,0);
    wprintw(win,"wakeup: 7:00 AM - 10:45 PM");
    wmove(win, 3,0);
    wprintw(win,"Entries completed: 2");
    mvwhline(win, 4, 0, ACS_HLINE, panel_width-4); 
    wprintw(win,"1. I've been devoting...");;
    wmove(win, 5,0);
    wprintw(win,"2.");
    wrefresh(win);
    wgetch(win);
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