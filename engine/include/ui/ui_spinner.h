/**
*** :: UI Spinner ::
***
***   Spinning loading icon
***
**/

#ifndef ui_spinner_h
#define ui_spinner_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec2 top_left;
  vec2 bottom_right;
  
  vec4 color;
  asset_hndl texture;
  
  float speed;
  float rotation;
  
  bool active;
} ui_spinner;

ui_spinner* ui_spinner_new();
void ui_spinner_delete(ui_spinner* s);

void ui_spinner_update(ui_spinner* s);
void ui_spinner_render(ui_spinner* s);


#endif
