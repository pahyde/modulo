#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "screen_model.h"
#include "entry_doc.h"

static void init_window_models(ScreenModel *screen_model, EntryDoc *entry_doc, int screen_h, int screen_w);
static void init_doc_model(DocModel *doc_model);
static void init_summary_model(SummaryModel *summary_model);

static void resize_doc_model(DocModel *doc_model, EntryDoc *entry_doc, int screen_h, int screen_w, bool small_width);
static void resize_doc_subwindows(DocModel *doc_model, EntryDoc *entry_doc);
static void resize_doc_header(DocModel *doc_model, EntryDoc *entry_doc);
static void resize_doc_entry_content(DocModel *doc_model);

static void resize_summary_model(SummaryModel *window_model, int screen_h, int screen_w, bool small_width);
static void resize_summary_subwindows(SummaryModel *summary_model);
static void resize_summary_logo(SummaryModel *summary_model);
static void resize_summary_entry_list_summary(SummaryModel *summary_model);

static int horizontal_margin(SubWindow *sub_win);
static int vertical_margin(SubWindow *sub_win);
static size_t line_wrap_count(size_t length, size_t width);

bool check_small_width(int width);

ScreenModel *create_screen_model(EntryDoc *entry_doc, int screen_h, int screen_w) {
    ScreenModel *screen_model = malloc(sizeof(ScreenModel));

    screen_model->height = screen_h;
    screen_model->width = screen_w;
    init_window_models(screen_model, entry_doc, screen_h, screen_w);
    return screen_model;
}

void free_screen_model(ScreenModel *screen_model) {
    free(screen_model);
}

void init_window_models(ScreenModel *screen_model, EntryDoc *entry_doc, int screen_h, int screen_w) {
    DocModel *doc_model = &screen_model->doc_model;
    SummaryModel *summary_model = &screen_model->summary_model;

    init_doc_model(doc_model);
    init_summary_model(summary_model);

    bool is_small_width = check_small_width(screen_w);
    resize_doc_model(doc_model, entry_doc, screen_h, screen_w, is_small_width);
    resize_summary_model(summary_model, screen_h, screen_w, is_small_width);
}

void init_doc_model(DocModel *doc_model) {
    doc_model->pos_y = 0; 

    SubWindow *entry_content = &doc_model->entry_content;
    entry_content->top = ENTRY_CONTENT_TOP;
    entry_content->bottom = ENTRY_CONTENT_BOTTOM;
    entry_content->left = ENTRY_CONTENT_LEFT;
    entry_content->right = ENTRY_CONTENT_RIGHT;
    entry_content->pos_x = 0;

    SubWindow *header = &doc_model->header;
    header->top = HEADER_TOP;
    header->bottom = HEADER_BOTTOM;
    header->left = HEADER_LEFT;
    header->right = HEADER_RIGHT;
    header->pos_y = 0;
    header->pos_x = 0;
    
    doc_model->content_update = true;
    doc_model->size_update = false;
}

void init_summary_model(SummaryModel *summary_model) {
    summary_model->pos_y = 0;
    summary_model->pos_x = 0;

    SubWindow *logo = &summary_model->logo;
    logo->top = LOGO_TOP;
    logo->bottom = LOGO_BOTTOM;
    logo->left = LOGO_LEFT;
    logo->right = LOGO_RIGHT;
    logo->pos_y = 0;
    logo->pos_x = 0;
    logo->height = LOGO_HEIGHT;

    SubWindow *entry_list_summary = &summary_model->entry_list_summary;
    entry_list_summary->top = SUMMARY_TOP;
    entry_list_summary->bottom = SUMMARY_BOTTOM;
    entry_list_summary->left = SUMMARY_LEFT;
    entry_list_summary->right = SUMMARY_RIGHT;
    entry_list_summary->pos_x = 0;
    entry_list_summary->pos_y = logo->height + vertical_margin(logo);

    summary_model->content_update = true;
    summary_model->size_update = false;
}

void resize_doc_model(DocModel *doc_model, EntryDoc *entry_doc, int screen_h, int screen_w, bool is_small_width) {
    doc_model->height = screen_h;
    doc_model->size_update = true;
    if (is_small_width) {
        doc_model->width = screen_w;
        doc_model->pos_x = 0;
    } else {
        doc_model->width = screen_w - SUMMARY_WIN_WIDTH;
        doc_model->pos_x = SUMMARY_WIN_WIDTH;
    }
    SubWindow *header = &doc_model->header;
    SubWindow *entry_content = &doc_model->entry_content;
    resize_doc_subwindows(doc_model, entry_doc);
}

void resize_doc_subwindows(DocModel *doc_model, EntryDoc *entry_doc) {
    // applying these in order matters
    // i.e. (header.height, header.width) => (entry_content.height, entry_conent.width)
    resize_doc_header(doc_model, entry_doc);
    resize_doc_entry_content(doc_model);
}

void resize_doc_header(DocModel *doc_model, EntryDoc *entry_doc) {
    SubWindow *header = &doc_model->header;

    header->width = doc_model->width - horizontal_margin(header);
    
    int header_height = 0;
    // This is a flex and honestly a terrible practice
    // easily could lead to undefined behavior I think
    // But coding this made it crystal clear to me that arrays, like structs, are values
    // -- not syntactic sugar for address/pointer
    char (*header_lines)[HEADER_MAX_LINES][HEADER_MAX_LINE_LENGTH] = &entry_doc->header.lines;
    int line_count = entry_doc->header.line_count;
    for (size_t i = 0; i < line_count; i++) {
        size_t length = strlen((*header_lines)[i]);
        header_height += line_wrap_count(length, header->width);
    }
    // underline
    header_height++;
    header->height = header_height;
    header->height = line_count;
}

size_t line_wrap_count(size_t length, size_t width) {
    size_t count = 1 + (length / width);
    if (length != 0 && length % width == 0) {
        count--;
    }
    return count;
}

void resize_doc_entry_content(DocModel *doc_model) {
    SubWindow *entry_content = &doc_model->entry_content;
    SubWindow *header = &doc_model->header;
    entry_content->pos_y = header->height + vertical_margin(header);
    entry_content->width = doc_model->width - horizontal_margin(entry_content);
    int container_height = doc_model->height - entry_content->pos_y;
    entry_content->height = container_height - vertical_margin(entry_content);
}

int horizontal_margin(SubWindow *sub_win) {
    return sub_win->left + sub_win->right;
}

int vertical_margin(SubWindow *sub_win) {
    return sub_win->top + sub_win->bottom;
}

void resize_summary_model(SummaryModel *summary_model, int screen_h, int screen_w, bool is_small_width) {
    int new_height = screen_h;
    int new_width = is_small_width ? 0 : SUMMARY_WIN_WIDTH;
    bool height_update = new_height != summary_model->height;
    bool width_update = new_width != summary_model->width;

    summary_model->size_update = height_update || width_update;
    summary_model->height = new_height;
    summary_model->width = new_width;
    resize_summary_subwindows(summary_model);
}
    
void resize_summary_subwindows(SummaryModel *summary_model) {
    resize_summary_logo(summary_model);
    resize_summary_logo(summary_model);
    resize_summary_entry_list_summary(summary_model);
}

void resize_summary_logo(SummaryModel *summary_model) {
    SubWindow *logo = &summary_model->logo;
    logo->width = summary_model->width - horizontal_margin(logo);
}

void resize_summary_entry_list_summary(SummaryModel *summary_model) {
    SubWindow *entry_list_summary = &summary_model->entry_list_summary;
    entry_list_summary->width = summary_model->width - horizontal_margin(entry_list_summary);
    int container_height = summary_model->height - entry_list_summary->pos_y;
    entry_list_summary->height = container_height - vertical_margin(entry_list_summary);
}

void screen_model_resize(ScreenModel *screen_model, EntryDoc *entry_doc, int height, int width) {
    bool is_small_width = check_small_width(width);
    resize_doc_model(&screen_model->doc_model, entry_doc, height, width, is_small_width);
    resize_summary_model(&screen_model->summary_model, height, width, is_small_width);
}

bool check_small_width(int width) {
    return width < WIDTH_BREAKPOINT;
}