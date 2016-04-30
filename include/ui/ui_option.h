#ifndef ui_option_h
#define ui_option_h

#include "ui/ui_button.h"

typedef struct ui_option {
  ui_button* label;
  int num_options;
  ui_button** options;
  bool active;
  int selected;
  void (*onselect)(struct ui_option*);
} ui_option;

ui_option* ui_option_new(void);
void ui_option_delete(ui_option* o);

void ui_option_set_active(ui_option* o, bool active);
void ui_option_move(ui_option* o, vec2 position);
void ui_option_set_options(ui_option* o, char* label, int num, char** values);
int  ui_option_get_selected(ui_option* o);
void ui_option_set_selected(ui_option* o, int selected);
void ui_option_set_onselect(ui_option* o, void(*onselect)(ui_option*));

void ui_option_deactivate(ui_option* o);
void ui_option_activate(ui_option* o);

void ui_option_event(ui_option* o, SDL_Event e);
void ui_option_update(ui_option* o);
void ui_option_render(ui_option* o);




#endif