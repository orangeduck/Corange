#ifndef ui_rectangle_h
#define ui_rectangle_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec2 top_left;
  vec2 bottom_right;
  
  vec4 color;
  asset_hndl texture;
  
  float border_size;
  vec4 border_color;
  
  bool active;
} ui_rectangle;

ui_rectangle* ui_rectangle_new();
void ui_rectangle_delete(ui_rectangle* rect);

void ui_rectangle_update(ui_rectangle* rect);
void ui_rectangle_render(ui_rectangle* rect);

void ui_rectangle_move(ui_rectangle* rect, vec2 pos);
void ui_rectangle_resize(ui_rectangle* rect, vec2 size);

bool ui_rectangle_contains_position(ui_rectangle* rect, vec2 pos);

#endif
