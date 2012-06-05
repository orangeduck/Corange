#ifndef ui_rectangle_h
#define ui_rectangle_h

#include <stdbool.h>

#include "assets/texture.h"

typedef struct {

  vector2 top_left;
  vector2 bottom_right;
  
  vector4 color;
  texture* texture;
  
  float border_size;
  vector4 border_color;
  
  bool active;
  
} ui_rectangle;

ui_rectangle* ui_rectangle_new();
void ui_rectangle_delete(ui_rectangle* rect);

void ui_rectangle_update(ui_rectangle* rect);
void ui_rectangle_render(ui_rectangle* rect);

void ui_rectangle_move(ui_rectangle* rect, vector2 pos);
void ui_rectangle_resize(ui_rectangle* rect, vector2 size);
void ui_rectangle_set_texture(ui_rectangle* rect, texture* t);

bool ui_rectangle_contains_position(ui_rectangle* rect, vector2 pos);

#endif
