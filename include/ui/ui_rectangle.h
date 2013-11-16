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
  float time;
  
  GLenum blend_src;
  GLenum blend_dst;
  
  bool active;
  
} ui_rectangle;

ui_rectangle* ui_rectangle_new();
void ui_rectangle_delete(ui_rectangle* r);

void ui_rectangle_event(ui_rectangle* r, SDL_Event e);
void ui_rectangle_update(ui_rectangle* r);
void ui_rectangle_render(ui_rectangle* r);

void ui_rectangle_move(ui_rectangle* r, vec2 pos);
void ui_rectangle_resize(ui_rectangle* r, vec2 size);
void ui_rectangle_set_texture(ui_rectangle* r, asset_hndl tex, int width, int height, bool tile);
void ui_rectangle_set_border(ui_rectangle* r, float size, vec4 color);
void ui_rectangle_set_color(ui_rectangle* r, vec4 color);
void ui_rectangle_set_glitch(ui_rectangle* r, float glitch);
vec2 ui_rectangle_center(ui_rectangle* r);
bool ui_rectangle_contains_point(ui_rectangle* r, vec2 pos);
void ui_rectangle_blend(ui_rectangle* r, GLenum blend_src, GLenum blend_dst);

vec2 ui_rectangle_position(ui_rectangle* r);
vec2 ui_rectangle_size(ui_rectangle* r);

#endif
