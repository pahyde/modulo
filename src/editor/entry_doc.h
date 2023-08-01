#ifndef ENTRY_DOC_H
#define ENTRY_DOC_H

#include <stdlib.h>
#include <stdbool.h>

#include "../modulo.h"

typedef struct Index {
    int i;
    int j;
} Index;
                                                    
#define INIT_LINE_LENGTH_CAP 128
#define INIT_LINE_COUNT_CAP 8

#define HEADER_MAX_LINES 8
#define HEADER_MAX_LINE_LENGTH 128

typedef struct Header {
    size_t line_count;
    char lines[HEADER_MAX_LINES][HEADER_MAX_LINE_LENGTH];
} Header;

typedef struct Line {
    size_t capacity;
    size_t length;
    char *chars;
} Line;

typedef struct EntryDoc {
    Header header;
    size_t capacity;
    size_t line_count;
    Line *lines;
    Index cursor;
    Index scroll;
} EntryDoc;

EntryDoc *create_entry_doc(Modulo *modulo);

void entry_doc_insert_char(EntryDoc *entry_doc, char c);
void entry_doc_backspace(EntryDoc *entry_doc);
void entry_doc_enter(EntryDoc *entry_doc);

void entry_doc_cursor_up(EntryDoc *entry_doc);
void entry_doc_cursor_down(EntryDoc *entry_doc);
void entry_doc_cursor_left(EntryDoc *entry_doc);
void entry_doc_cursor_right(EntryDoc *entry_doc);

Index entry_doc_get_effective_cursor(EntryDoc *entry_doc);
Line *entry_doc_get_line(EntryDoc *entry_doc, size_t index);

void entry_doc_clear(Modulo *modulo, EntryDoc *entry_doc);
void free_entry_doc(EntryDoc *entry_doc);


#endif