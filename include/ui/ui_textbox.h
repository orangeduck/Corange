#ifndef ui_textbox_h
#define ui_textbox_h

#include "ui/ui_rectangle.h"
#include "ui/ui_text.h"

typedef struct {
  
  ui_rectangle* outer;
  ui_rectangle* inner;
  
  ui_text* contents;
  ui_text* label;
  
  bool password;
  int max_chars;
  
  bool selected;
  bool active;
  bool enabled;
  
} ui_textbox;

ui_textbox* ui_textbox_new();
void ui_textbox_delete(ui_textbox* tb);

void ui_textbox_set_password(ui_textbox* tb, bool password);
void ui_textbox_set_max_chars(ui_textbox* tb, int l);
void ui_textbox_addchar(ui_textbox* tb, char c);
void ui_textbox_rmchar(ui_textbox* tb);

void ui_textbox_move(ui_textbox* tb, vec2 pos);
void ui_textbox_resize(ui_textbox* tb, vec2 size);

void ui_textbox_set_font(ui_textbox* tb, asset_hndl f);
void ui_textbox_set_label(ui_textbox* tb, char* label);
void ui_textbox_set_contents(ui_textbox* tb, char* label);

void ui_textbox_disable(ui_textbox* tb);
void ui_textbox_enable(ui_textbox* tb);

void ui_textbox_event(ui_textbox* tb, SDL_Event e);
void ui_textbox_update(ui_textbox* tb);
void ui_textbox_render(ui_textbox* tb);

bool ui_textbox_contains_point(ui_textbox* tb, vec2 p);

#endif