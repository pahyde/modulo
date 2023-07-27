#ifndef ENTRY_DOC_H
#define ENTRY_DOC_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct Index {
    int i;
    int j;
} Index;
                                                    
#define INIT_LINE_LENGTH_CAP 128
#define INIT_LINE_COUNT_CAP 8

typedef struct Line {
    size_t capacity;
    size_t length;
    char *chars;
} Line;

typedef struct EntryDoc {
    size_t capacity;
    size_t line_count;
    Line *lines;
    Index cursor;
    Index scroll;
} EntryDoc;


void entry_doc_insert_char(EntryDoc *entry_doc, char c);
void entry_doc_backspace(EntryDoc *entry_doc);
void entry_doc_enter(EntryDoc *entry_doc);

void entry_doc_cursor_up(EntryDoc *entry_doc, bool limit_range);
void entry_doc_cursor_down(EntryDoc *entry_doc, bool limit_range);
void entry_doc_cursor_left(EntryDoc *entry_doc, bool limit_range);
void entry_doc_cursor_right(EntryDoc *entry_doc, bool limit_range);


#endif