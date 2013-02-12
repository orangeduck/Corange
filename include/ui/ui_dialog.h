/**
*** :: UI Dialog ::
***
***
**/

#ifndef ui_dialog_h
#define ui_dialog_h

#include "cengine.h"

#include "ui/ui_text.h"
#include "ui/ui_button.h"

extern int dialog_count;

typedef struct {
  
  ui_button* back;
  ui_text* title;
  ui_text* contents;
  
  bool single_button;
  
  ui_button* left;
  ui_button* right;

} ui_dialog;

ui_dialog* ui_dialog_new();
void ui_dialog_delete(ui_dialog* d);

void ui_dialog_set_single_button(ui_dialog* d, bool single);
void ui_dialog_set_title(ui_dialog* d, char* title);
void ui_dialog_set_contents(ui_dialog* d, char* contents);
void ui_dialog_set_button_left(ui_dialog* d, char* left, void (*onleft)(ui_button*));
void ui_dialog_set_button_right(ui_dialog* d, char* right, void (*onright)(ui_button*));
void ui_dialog_set_font(ui_dialog* d, asset_hndl fnt);

void ui_dialog_event(ui_dialog* d, SDL_Event e);
void ui_dialog_update(ui_dialog* d);
void ui_dialog_render(ui_dialog* d);

#endif