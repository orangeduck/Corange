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

static int text_align_left = 0;
static int text_align_right = 1;
static int text_align_center = 2;

static int text_align_top = 0;
static int text_align_bottom = 1;

typedef struct {
  
  /* private */
  char* string;
  int buffersize;
  
  GLuint positions_buffer;
  GLuint texcoords_buffer;
  
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
  
  bool active;
  
} ui_text;

ui_text* ui_text_new();
ui_text* ui_text_new_string(char* string);
void ui_text_delete(ui_text* text);

void ui_text_draw(ui_text* text);
void ui_text_draw_string(ui_text* text, char* string);

void ui_text_update(ui_text* text);
void ui_text_render(ui_text* text);

bool ui_text_contains_position(ui_text* text, vec2 position);

#endif
