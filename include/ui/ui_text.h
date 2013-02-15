/**
*** :: UI Text ::
***
***   A text object in the UI.
***   
***   To change text properties access the struct
***   Then call "ui_text_draw" to redraw the text
***   vertex buffer. 
***
**/

#ifndef ui_text_h
#define ui_text_h

#include "cengine.h"
#include "casset.h"

enum {
  text_align_left   = 0,
  text_align_right  = 1,
  text_align_center = 2,
};

enum {
  text_align_top    = 0,
  text_align_bottom = 1,
};

typedef struct {
  
  /* private */
  char* string;
  
  GLuint positions_buffer;
  GLuint texcoords_buffer;
  GLuint colors_buffer;
  
  int num_positions;
  int num_texcoords;
  
  vec2 top_left;
  vec2 bottom_right;
  
  /* public */
  asset_hndl font;  
  
  vec2 position;
  vec2 scale;
  vec4 color;
  
  int halign;
  int valign;
  
  float line_spacing;
  float char_spacing;
  float rotation;
  
  float line_length;
  
  bool active;
  
} ui_text;

ui_text* ui_text_new();
ui_text* ui_text_new_string(char* string);
void ui_text_delete(ui_text* text);

void ui_text_move(ui_text* text, vec2 pos);
void ui_text_set_font(ui_text* text, asset_hndl font);
void ui_text_set_color(ui_text* text, vec4 color);
void ui_text_set_scale(ui_text* text, vec2 scale);
void ui_text_align(ui_text* text, int halign, int valign);

void ui_text_draw(ui_text* text);
void ui_text_draw_string(ui_text* text, char* string);

void ui_text_event(ui_text* text, SDL_Event e);
void ui_text_update(ui_text* text);
void ui_text_render(ui_text* text);

bool ui_text_contains_point(ui_text* text, vec2 position);

#endif
