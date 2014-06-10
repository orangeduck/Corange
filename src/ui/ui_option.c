
#include "ui/ui_option.h"

ui_option* ui_option_new(void) {

  ui_option* o = malloc(sizeof(ui_option));
  o->label = ui_button_new();
  ui_button_disable(o->label);
  ui_button_resize(o->label, vec2_new(150, 30));
  
  o->num_options = 0;
  o->options = NULL;
  o->active = true;
  o->selected = -1;
  o->onselect = NULL;
  
  return o;
}

void ui_option_delete(ui_option* o) {

  ui_button_delete(o->label);

  for (int i = 0; i < o->num_options; i++) {
    ui_button_delete(o->options[i]);
  }
  free(o->options);
  free(o);
  
}

void ui_option_move(ui_option* o, vec2 position) {
  
  ui_button_move(o->label, position);
  
  for (int i = 0; i < o->num_options; i++) {
    position = vec2_add(position, vec2_new(180, 0));
    ui_button_move(o->options[i], position);
  }
  
}

static void option_value_select(ui_button* b, void* data) {
  
  ui_option* o = data;
  
  for (int i = 0; i < o->num_options; i++) {
    if (o->options[i] == b) { ui_option_set_selected(o, i); break; }
  }

  if (o->onselect) { o->onselect(o); }
}

void ui_option_set_options(ui_option* o, char* label, int num, char** values) {

  ui_button_set_label(o->label, label);

  for (int i = 0; i < o->num_options; i++) {
    ui_button_delete(o->options[i]);
  }
  
  o->num_options = num;
  o->options = realloc(o->options, sizeof(ui_button*) * o->num_options);
  
  for (int i = 0; i < o->num_options; i++) {
    ui_button* ob = ui_button_new();
    ui_button_resize(ob, vec2_new(150, 30));
    ui_button_set_label(ob, values[i]);
    ui_button_set_enabled(ob, i != o->selected);
    ui_button_set_onclick(ob, option_value_select);
    ui_button_set_onclick_data(ob, o);
    o->options[i] = ob;
  }

  ui_option_move(o, ui_button_position(o->label));

}

int ui_option_get_selected(ui_option* o) {
  return o->selected;
}

void ui_option_set_selected(ui_option* o, int selected) {

  o->selected = selected;

  for (int i = 0; i < o->num_options; i++) {
    ui_button_set_enabled(o->options[i], i != o->selected);
  }

}

void ui_option_set_active(ui_option* o, bool active) {
  o->active = active;
}

void ui_option_deactivate(ui_option* o) {
  o->active = false;
}

void ui_option_activate(ui_option* o) {
  o->active = true;
}

void ui_option_set_onselect(ui_option* o, void(*onselect)(ui_option*)) {
  o->onselect = onselect;
}

void ui_option_event(ui_option* o, SDL_Event e) {
  
  if (!o->active) { return; }
  
  ui_button_event(o->label, e);
  for (int i = 0; i < o->num_options; i++) {
    ui_button_event(o->options[i], e);
  }
  
}


void ui_option_update(ui_option* o) {

  if (!o->active) { return; }

  ui_button_update(o->label);
  for (int i = 0; i < o->num_options; i++) {
    ui_button_update(o->options[i]);
  }

}

void ui_option_render(ui_option* o) {

  if (!o->active) { return; }

  ui_button_render(o->label);
  for (int i = 0; i < o->num_options; i++) {
    ui_button_render(o->options[i]);
  }

}
