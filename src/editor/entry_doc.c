#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "entry_doc.h"

static void entry_doc_insert_line(EntryDoc *entry_doc, Line *line, size_t index);
static Line entry_doc_remove_line(EntryDoc *entry_doc, size_t index);
static void entry_doc_move_cursor(EntryDoc *entry_doc, int i, int j);

static Line create_line(size_t capacity);
static void line_insert_char(Line *line, char c, size_t index);
static void line_cat(Line *dest, Line *src);
static Line line_slice(Line *line, size_t index);
static void line_remove_char(Line *line, size_t index);
static Line create_empty_line();

static void check_line_capacity(EntryDoc *entry_doc);
static void check_char_capacity(Line *line, size_t required);
static void free_line(Line *line);

EntryDoc *create_entry_doc() {
    EntryDoc *entry_doc = malloc(sizeof(EntryDoc));
    entry_doc->capacity = INIT_LINE_LENGTH_CAP;
    entry_doc->line_count = 1;
    entry_doc->lines = malloc(INIT_LINE_LENGTH_CAP * sizeof(Line));
    entry_doc->lines[0] = create_empty_line();
    entry_doc->cursor = (Index) { .i = 0, .j = 0 };
    entry_doc->scroll = (Index) { .i = 0, .j = 0 };
}

Line create_empty_line() {
    Line empty_line = {
        .capacity = INIT_LINE_LENGTH_CAP,
        .length = 0,
        .chars = malloc(INIT_LINE_LENGTH_CAP * sizeof(char))
    };
    return empty_line;
}

void line_insert_char(Line *line, char c, size_t index) {
    check_char_capacity(line, 1);
    for (size_t i = line->length; i > index; i--) {
        line->chars[i] = line->chars[i-1];
    }
    line->chars[index] = c;
    line->length++;
}

void check_char_capacity(Line *line, size_t required) {
    size_t actual_cap = line->capacity;
    size_t required_cap = line->length + required;
    if (actual_cap >= required_cap) {
        return;
    }
    size_t new_cap = actual_cap;
    while (new_cap < required_cap) {
        new_cap *= 2;
    } 
    line->chars = realloc(line->chars, new_cap * sizeof(char));
    line->capacity = new_cap;
}

void line_cat(Line *dest, Line *src) {
    check_char_capacity(dest, src->length);
    memcpy(dest + dest->length, src, src->length * sizeof(char));
    dest->length += src->length;
}

/*
returns a slice (copy) from given index to end of line
does not mutate mutate/remove characters in original

Could be more general with params: start_index, end_index
*/
Line line_slice(Line *line, size_t index) {
    size_t slice_length = line->length - index;
    size_t slice_cap = INIT_LINE_LENGTH_CAP;
    while (slice_cap <= slice_length) {
        slice_cap *= 2;
    }
    Line slice = create_line(slice_cap);
    memcpy(slice.chars, &line->chars[index], slice_length * sizeof(char));
    slice.length = slice_length;
    return slice;
}

Line create_line(size_t capacity) {
    return (Line) {
        .capacity = capacity,
        .length = 0,
        .chars = malloc(capacity * sizeof(char))
    };
}

void entry_doc_insert_char(EntryDoc *entry_doc, char c) {
    Index *cursor = &entry_doc->cursor;
    Line *line = entry_doc_get_line(entry_doc, cursor->i);
    line_insert_char(line, c, cursor->j);
    entry_doc_cursor_right(entry_doc, false);
}

void entry_doc_backspace(EntryDoc *entry_doc) {
    Index *cursor = &entry_doc->cursor;

    if (cursor->i == 0 && cursor->j == 0) {
        // top of document
        return;
    }
    if (cursor->j == 0) {
        // line start
        Line removed = entry_doc_remove_line(entry_doc, cursor->i);
        Line *prev_line = entry_doc_get_line(entry_doc, cursor->i-1);
        entry_doc_move_cursor(entry_doc, cursor->i-1, prev_line->length);
        line_cat(prev_line, &removed);
        free_line(&removed);
        return;
    } 
    // delete char
    Line *line = entry_doc_get_line(entry_doc, cursor->i);
    line_remove_char(line, cursor->j);
    entry_doc_cursor_left(entry_doc, false);
}

void line_remove_char(Line *line, size_t index) {
    if (index >= line->length || index < 0) {
        fprintf(stderr, "Can't remove character at index %zu from line of length %zu\n", index, line->length);
        exit(EXIT_FAILURE);
    }
    for (size_t i = index+1; i < line->length; i++) {
        line->chars[i-1] = line->chars[i];
    }
    line->length--;
}

void entry_doc_enter(EntryDoc *entry_doc) {
    Index *cursor = &entry_doc->cursor;

    Line *line = entry_doc_get_line(entry_doc, cursor->i);
    Line slice = line_slice(line, cursor->j);
    line->length = cursor->j;
    entry_doc_insert_line(entry_doc, &slice, cursor->i+1);
    entry_doc_move_cursor(entry_doc, cursor->i+1, 0);
}


void entry_doc_insert_line(EntryDoc *entry_doc, Line *line, size_t index) {
    size_t line_count = entry_doc->line_count;
    if (index > line_count || index < 0) {
        fprintf(stderr, "Can't insert line at index %zu into document with line_count %zu\n", index, line_count);
        exit(EXIT_FAILURE);
    }
    check_line_capacity(entry_doc);
    for (size_t i = entry_doc->line_count; i > index; i--) {
        entry_doc->lines[i] = entry_doc->lines[i-1];
    }
    entry_doc->lines[index] = *line;
    entry_doc->line_count++;
}

void check_line_capacity(EntryDoc *entry_doc) {
    size_t cap = entry_doc->capacity;
    size_t required_cap = entry_doc->line_count+1;
    if (required_cap <= cap) {
        return;
    }
    size_t new_cap = cap * 2; 
    entry_doc->lines = realloc(entry_doc->lines, new_cap * sizeof(Line));
    entry_doc->capacity = new_cap;
}

Line *entry_doc_get_line(EntryDoc *entry_doc, size_t index) {
    size_t line_count = entry_doc->line_count;
    if (index >= line_count) {
        fprintf(stderr, "Can't get line at index %zu from document with %zu lines\n", index, line_count);
        exit(EXIT_FAILURE);
    }
    return &entry_doc->lines[index];
}

Line entry_doc_remove_line(EntryDoc *entry_doc, size_t index) {
    size_t line_count = entry_doc->line_count;
    if (index >= line_count || index < 0) {
        fprintf(stderr, "Can't remove line at index %zu from document with line count %zu\n", index, line_count);
        exit(EXIT_FAILURE);
    }
    Line removed = *entry_doc_get_line(entry_doc, index);
    for (size_t i = index+1; i < line_count; i++) {
        entry_doc->lines[i-1] = entry_doc->lines[i];
    }
    entry_doc->line_count--;
    // calling function must free chars in removed
    return removed;
}

/*
Cursor move functions corresponding to arrow key inputs

implementation allows for phantom j/column index
when moving cursor up through short lines i.e.

line 1: "Lorem ipsum dolor s|it amet, consectetur adipiscing 
                            ^
line 2: elit, sed do        |
                            ^
line 3: eiusmod tempor incid|idunt ut labo

The view will check the actually line length
and render the cursor at MIN(line.length, cursor.j index)

*/
void entry_doc_cursor_up(EntryDoc *entry_doc, bool limit_range) {
    Index *cursor = &entry_doc->cursor;
    if (limit_range && cursor->i == 0) {
        return;
    }
    cursor->i--;
}

void entry_doc_cursor_down(EntryDoc *entry_doc, bool limit_range) {
    Index *cursor = &entry_doc->cursor;
    if (limit_range && cursor->i == entry_doc->line_count-1) {
        return;
    }
    cursor->i++;
}

void entry_doc_cursor_left(EntryDoc *entry_doc, bool limit_range) {
    Index *cursor = &entry_doc->cursor;
    Line *line = &entry_doc->lines[cursor->i];
    if (limit_range && cursor->j == 0) {
        return;
    }
    // handle cursor j-index out of range for current line
    cursor->j = (cursor->j < line->length) ? cursor->j-1 : line->length-1; 
}

void entry_doc_cursor_right(EntryDoc *entry_doc, bool limit_range) {
    Index *cursor = &entry_doc->cursor;
    Line *line = &entry_doc->lines[cursor->i];
    if (limit_range && cursor->j == line->length) {
        return;
    }
    cursor->j++;
}

/*
Returns the physical cursor location in the document
limits the logical cursor (entry_doc->cursor) column index by the current line length
*/
Index entry_doc_get_cursor(EntryDoc *entry_doc) {
    Index cursor = entry_doc->cursor;
    size_t line_length = entry_doc_get_line(entry_doc, cursor.i)->length;
    if (cursor.j > line_length) {
        cursor.j = line_length;
    }
    return cursor;
}

// used by enter and backspace. 
// should never have cursor out of range for line (unlike single-step functions above)
void entry_doc_move_cursor(EntryDoc *entry_doc, int i, int j) {
    size_t line_count = entry_doc->line_count;
    if (i < 0 || i >= line_count) {
        fprintf(stderr, "Can't move cursor to position i: %d, j: %d in document with line count: %zu\n", i, j, line_count);
        exit(EXIT_FAILURE);
    }
    size_t line_length = entry_doc->lines[i].length;
    if (j < 0 || j > line_length) {
        fprintf(stderr, "Can't move cursor to position i: %d, j: %d in line of length: %zu\n", i, j, line_length);
        exit(EXIT_FAILURE);
    }
    entry_doc->cursor.i = i;
    entry_doc->cursor.j = j;
}

void entry_doc_clear(EntryDoc *entry_doc) {
    entry_doc->cursor = (Index) { .i = 0, .j = 0 };
    entry_doc->scroll = (Index) { .i = 0, .j = 0 };
    for (size_t i = 1; i < entry_doc->line_count; i++) {
        free_line(entry_doc_get_line(entry_doc, i));
    }
    entry_doc->line_count = 1;
    entry_doc_get_line(entry_doc, 0)->length = 0;
}

void free_entry_doc(EntryDoc *entry_doc) {
    for (size_t i = 0; i < entry_doc->line_count; i++) {
        free_line(entry_doc_get_line(entry_doc, i));
    }
    free(entry_doc->lines);
    free(entry_doc);
}

void free_line(Line *line) {
    free(line->chars);
}