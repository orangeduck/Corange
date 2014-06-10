#ifndef ui_style_h
#define ui_style_h

#include "cengine.h"
#include "ui/ui_text.h"

typedef struct {
  
  /* Box */
  
  fpath box_back_image;
  int   box_back_width;
  int   box_back_height;
  bool  box_back_tile;
  int   box_back_border_size;
  vec4  box_back_border_color;
  float box_glitch;
  
  vec4 box_text_color;
  vec4 box_label_color;
  int  box_text_halign;
  int  box_text_valign;
  vec4 box_up_color;
  vec4 box_down_color;
  vec4 box_inset_color;
  
  /* Text  */
  
  fpath text_font;
  vec4  text_color;
  vec2  text_scale;
  
  /* Spinner */
  
  fpath spinner_image;
  float spinner_speed;

} ui_style;

/* Current */

extern ui_style* ui_style_current;

/* Defaults */

extern ui_style ui_style_corange;
extern ui_style ui_style_hunt;

#endif