#ifndef SCREEN_MODEL_H
#define SCREEN_MODEL_H

#include <stdbool.h>

#define SUMMARY_WIN_WIDTH 28

typedef struct WindowModel {
    int height;
    int width;
    int top;   // going with css absolute-positioning mental model, feels right.
    int left;
    bool is_hidden;
} WindowModel;

typedef struct ScreenModel {
    int height;
    int width;
    WindowModel doc_model;
    WindowModel summary_model;
    bool resize
} ScreenModel;

ScreenModel *create_screen_model();

void screen_model_resize(ScreenModel *screen_model, int height, int width);
bool screen_model_is_resize(ScreenModel *screen_model);
void screen_model_set_resize(ScreenModel *screen_model, bool flag);

#endif