#ifndef SCREEN_MODEL_H
#define SCREEN_MODEL_H

#include <stdbool.h>

#include "entry_doc.h"

#define WIDTH_BREAKPOINT 56

#define SUMMARY_WIN_WIDTH 28

#define SUMMARY_WIN_PADDING 2
#define DOC_WIN_PADDING 2
#define DOC_WIN_HEADER_HEIGHT 

// What is this, CSS ??
#define HEADER_TOP 2
#define HEADER_BOTTOM 2
#define HEADER_LEFT 2
#define HEADER_RIGHT 2

#define ENTRY_CONTENT_TOP 2
#define ENTRY_CONTENT_BOTTOM 2
#define ENTRY_CONTENT_LEFT 2
#define ENTRY_CONTENT_RIGHT 2

#define LOGO_TOP 2
#define LOGO_BOTTOM 2
#define LOGO_LEFT 2
#define LOGO_RIGHT 2
#define LOGO_HEIGHT 8

#define SUMMARY_TOP 2
#define SUMMARY_BOTTOM 2
#define SUMMARY_LEFT 2
#define SUMMARY_RIGHT 2

typedef struct SubWindow {
    int height;
    int width;
    int pos_y;  
    int pos_x;  
    int top;   // going with css absolute-positioning mental model, feels right.
    int bottom;
    int left;
    int right;
} SubWindow;

typedef struct SummaryModel {
    int height;
    int width;
    int pos_y;   
    int pos_x;
    SubWindow logo;
    SubWindow entry_list_summary;
    bool content_update;
    bool size_update;
} SummaryModel;

typedef struct DocModel {
    int height;
    int width;
    int pos_y;   
    int pos_x;
    SubWindow header;
    SubWindow entry_content;
    bool content_update;
    bool size_update;
} DocModel;

typedef struct ScreenModel {
    int height;
    int width;
    DocModel doc_model;
    SummaryModel summary_model;
} ScreenModel;

ScreenModel *create_screen_model(EntryDoc *entry_doc, int screen_h, int screen_w);
void free_screen_model(ScreenModel *screen_model);

void screen_model_resize(ScreenModel *screen_model, EntryDoc *entry_doc, int height, int width);
bool screen_model_is_resize(ScreenModel *screen_model);
void screen_model_set_resize(ScreenModel *screen_model, bool flag);

#endif