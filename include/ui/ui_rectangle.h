/**
*** :: UI Rectangle ::
***
***   Basic rectangle object
***
**/

#ifndef ui_rectangle_h
#define ui_rectangle_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec2 top_left;
  vec2 bottom_right;
  
  vec4 color;
  asset_hndl texture;
  int texture_width;
  int texture_height;
  bool texture_tile;
  
  float border_size;
  vec4 border_color;
  
  float glitch;
  
  bool active;
} ui_rectangle;

ui_rectangle* ui_rectangle_new();
void ui_rectangle_delete(ui_rectangle* rect);

void ui_rectangle_update(ui_rectangle* rect);
void ui_rectangle_render(ui_rectangle* rect);

void ui_rectangle_move(ui_rectangle* rect, vec2 pos);
void ui_rectangle_resize(ui_rectangle* rect, vec2 size);
void ui_rectangle_set_texture(ui_rectangle* rect, asset_hndl tex, int width, int height, bool tile);
void ui_rectangle_set_border(ui_rectangle* rect, float size, vec4 color);
void ui_rectangle_set_color(ui_rectangle* rect, vec4 color);
void ui_rectangle_set_glitch(ui_rectangle* rect, float glitch);
vec2 ui_rectangle_center(ui_rectangle* rect);
bool ui_rectangle_contains_point(ui_rectangle* rect, vec2 pos);

#endif
