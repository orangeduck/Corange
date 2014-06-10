#ifndef ui_slider_h
#define ui_slider_h

#include "ui/ui_rectangle.h"
#include "ui/ui_button.h"

typedef struct {
  
  ui_button* label;

  ui_rectangle* slot;
  ui_rectangle* bar;  

  bool pressed;
  bool active;
  float amount;
  
} ui_slider;

ui_slider* ui_slider_new(void);
void ui_slider_delete(ui_slider* s);

void ui_slider_set_label(ui_slider* s, char* label);
void ui_slider_move(ui_slider* s, vec2 position);
void ui_slider_set_amount(ui_slider* s, float amount);
float ui_slider_get_amount(ui_slider* s);
void ui_slider_set_active(ui_slider* s, bool active);

void ui_slider_deactivate(ui_slider* s);
void ui_slider_activate(ui_slider* s);

void ui_slider_event(ui_slider* s, SDL_Event e);
void ui_slider_update(ui_slider* s);
void ui_slider_render(ui_slider* s);


#endif