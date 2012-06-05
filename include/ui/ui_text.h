#ifndef ui_text_h
#define ui_text_h

#include <stdbool.h>

#include "vector.h"

#include "assets/font.h"

static int text_align_left = 0;
static int text_align_right = 1;
static int text_align_center = 2;
static int text_align_top = 0;
static int text_align_bottom = 1;

/*
  After editing any properties of a ui_text in this struct, please call the update function
  This will re-draw the quads needed for rendering the updated text.
*/

typedef struct {
  
  /* Don't change manually */
  
  char* string;
  int buffersize;
  
  GLuint positions_buffer;
  GLuint texcoords_buffer;
  
  int num_positions;
  int num_texcoords;
  
  vector2 top_left;
  vector2 bottom_right;
  
  /* Do change manually, then run update_properties function */
  
  font* font;  
  
  vector2 position;
  vector2 scale;
  vector4 color;
  
  int alignment;
  int vertical_alignment;
  
  float line_spacing;
  float char_spacing;
  float rotation;
  
  bool active;
  
} ui_text;

ui_text* ui_text_new();
ui_text* ui_text_new_string(char* string);
void ui_text_delete(ui_text* text);

void ui_text_update_string(ui_text* text, char* string);
void ui_text_update_properties(ui_text* text);

void ui_text_update(ui_text* text);
void ui_text_render(ui_text* text);

bool ui_text_contains_position(ui_text* text, vector2 position);

#endif
