/**
*** :: UI Button ::
***
***   Clickable UI button that retains state
***
**/

#ifndef ui_button_h
#define ui_button_h

#include "cengine.h"
#include "ui/ui_text.h"
#include "ui/ui_rectangle.h"

typedef struct {

  ui_rectangle* back;
  ui_text* label;
  
  vec4 up_color;
  vec4 down_color;
  
  void (*onclick)(void);
  
  bool active;
  bool enabled;
  bool pressed;
} ui_button;

ui_button* ui_button_new();
void ui_button_delete(ui_button* b);

void ui_button_move(ui_button* b, vec2 pos);
void ui_button_resize(ui_button* b, vec2 size);
void ui_button_set_label(ui_button* b, char* label);
void ui_button_set_font(ui_button* b, asset_hndl f);
void ui_button_set_texture(ui_button* b, asset_hndl t);
void ui_button_set_onclick(ui_button* b, void(*onclick)(void));
void ui_button_disable(ui_button* b);
void ui_button_enable(ui_button* b);

void ui_button_event(ui_button* b, SDL_Event e);
void ui_button_update(ui_button* b);
void ui_button_render(ui_button* b);

bool ui_button_contains_point(ui_button* b, vec2 pos);

#endif
