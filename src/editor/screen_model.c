#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "screen_model.h"

static void init_window_models(ScreenModel *scree_model, int screen_h, int screen_w);
static void init_doc_model(WindowModel *window_model);
static void init_summary_model(WindowModel *window_model);
static void resize_doc_model(WindowModel *doc_model, int screen_h, int screen_w, bool small_width);
static void resize_summary_model(WindowModel *window_model, int screen_h, int screen_w, bool small_width);

bool check_small_width(int width);

ScreenModel *create_screen_model(int screen_h, int screen_w) {
    ScreenModel *screen_model = malloc(sizeof(ScreenModel));

    screen_model->height = screen_h;
    screen_model->width = screen_w;
    init_window_models(screen_model, screen_h, screen_w);
    return screen_model;
}

void free_screen_model(ScreenModel *screen_model) {
    free(screen_model);
}

void init_window_models(ScreenModel *screen_model, int screen_h, int screen_w) {
    WindowModel *doc_model = &screen_model->doc_model;
    WindowModel *summary_model = &screen_model->summary_model;

    init_doc_model(doc_model);
    init_summary_model(summary_model);

    bool is_small_width = check_small_width(screen_w);
    resize_doc_model(doc_model, screen_h, screen_w, is_small_width);
    resize_summary_model(summary_model, screen_h, screen_w, is_small_width);
}

void init_doc_model(WindowModel *doc_model) {
    doc_model->top = 0; 
    doc_model->content_update = true;
}

void init_summary_model(WindowModel *summary_model) {
    summary_model->top = 0;
    summary_model->left = 0;
    summary_model->content_update = true;
}

void resize_doc_model(WindowModel *doc_model, int screen_h, int screen_w, bool is_small_width) {
    doc_model->height = screen_h;
    if (is_small_width) {
        doc_model->width = screen_w;
        doc_model->left = 0;
    } else {
        doc_model->width = screen_w - SUMMARY_WIN_WIDTH;
        doc_model->left = SUMMARY_WIN_WIDTH;
    }
}

void resize_summary_model(WindowModel *summary_model, int screen_h, int screen_w, bool is_small_width) {
    summary_model->height = screen_h;
    summary_model->width = is_small_width ? 0 : SUMMARY_WIN_WIDTH;
}

void screen_model_resize(ScreenModel *screen_model, int height, int width) {
    bool is_small_width = check_small_width(width);
    resize_doc_model(&screen_model->doc_model, height, width, is_small_width);
    resize_summary_model(&screen_model->summary_model, height, width, is_small_width);
}

bool check_small_width(int width) {
    return width < SUMMARY_WIN_WIDTH + 32;
}