#ifndef ui_button_h
#define ui_button_h

#include "bool.h"
#include "texture.h"

#include "ui_text.h"

typedef struct {

  vector2 top_left;
  vector2 bottom_right;
  
  ui_text* label;
  
  vector4 color;
  vector4 pressed_color;
  texture* texture;
  
  float border_size;
  vector4 border_color;
  
  bool active;
  bool enabled;
  bool pressed;
  
} ui_button;

ui_button* ui_button_new();
void ui_button_delete(ui_button* b);

void ui_button_move(ui_button* b, vector2 pos);
void ui_button_set_label(ui_button* b, char* label);
void ui_button_update(ui_button* b);
void ui_button_render(ui_button* b);

void ui_button_set_texture(ui_button* b, texture* t);

bool ui_button_contains_position(ui_button* b, vector2 pos);

#endif
