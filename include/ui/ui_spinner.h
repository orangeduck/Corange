#ifndef ui_spinner_h
#define ui_spinner_h

#include <stdbool.h>

#include "vector.h"

#include "assets/texture.h"

typedef struct {
  
  vector2 top_left;
  vector2 bottom_right;
  vector4 color;
  texture* texture;
  float speed;
  float rotation;
  
  bool active;
  
} ui_spinner;

ui_spinner* ui_spinner_new();
void ui_spinner_delete(ui_spinner* s);

void ui_spinner_update(ui_spinner* s);
void ui_spinner_render(ui_spinner* s);


#endif
