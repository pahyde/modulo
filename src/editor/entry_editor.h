#ifndef ENTRY_EDITOR_H
#define ENTRY_EDITOR_H

#include <ncurses.h>

#include "../modulo.h"
#include "../filesystem.h"

/*
typedef struct EditorEvent {
    type: ENTRY_DELIM, EXIT, RESIZE, KEY_PRESS
    input:  
} EditorEvent;
*/

typedef enum EventType {
    ENTRY_SUBMIT,
    EXIT,
    RESIZE,
    BACKSPACE,
    ENTER,
    CURSOR_MOVE, 
    CHAR_INPUT,
    NONE
} EventType;

typedef struct EditorEvent {
    EventType type;
    int input;
} EditorEvent;

void entry_editor_start(Modulo *modulo, OSContext *c);

#endif