#include "ui/ui_slider.h"
#include "ui/ui_style.h"

ui_slider* ui_slider_new(void) {
  ui_slider* s = malloc(sizeof(ui_slider));
  
  s->label = ui_button_new();
  ui_button_disable(s->label);
  ui_button_resize(s->label, vec2_new(150, 30));
  
  s->slot = ui_rectangle_new();
  ui_rectangle_resize(s->slot, vec2_new(300, 4));
  ui_rectangle_set_texture(s->slot, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(s->slot,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
    ui_rectangle_set_glitch(s->slot, ui_style_current->box_glitch);
  ui_rectangle_set_color(s->slot, vec4_black());
  
  s->bar = ui_rectangle_new();
  ui_rectangle_resize(s->bar, vec2_new(8, 30));
  ui_rectangle_set_texture(s->bar, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(s->bar,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
  ui_rectangle_set_glitch(s->bar, ui_style_current->box_glitch);
  
  return s;
}

void ui_slider_delete(ui_slider* s) {
  ui_button_delete(s->label);
  ui_rectangle_delete(s->slot);
  ui_rectangle_delete(s->bar);
}

void ui_slider_set_label(ui_slider* s, char* label) {
  ui_button_set_label(s->label, label);
}

void ui_slider_move(ui_slider* s, vec2 position) {
  ui_button_move(s->label, position);
  ui_rectangle_move(s->slot, vec2_add(ui_button_position(s->label), vec2_new(180, 12)));
  ui_rectangle_move(s->bar,  vec2_add(ui_button_position(s->label), vec2_new(180,  0)));
}

void ui_slider_set_amount(ui_slider* s, float amount) {
  s->amount = amount;
  ui_rectangle_move(s->bar, vec2_add(ui_button_position(s->label), vec2_new(180 + 292 * (1.0 - amount), 0)));
}

float ui_slider_get_amount(ui_slider* s) {
  return s->amount;
}

void ui_slider_set_active(ui_slider* s, bool active) {
  s->active = active;
}

void ui_slider_deactivate(ui_slider* s) {
  s->active = false;
}

void ui_slider_activate(ui_slider* s) {
  s->active = true;
}

void ui_slider_event(ui_slider* s, SDL_Event e) {
  
  if (!s->active) { return; }
  
  if (e.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_point(s->bar, vec2_new(e.motion.x, e.motion.y))) {
      s->pressed = true;
    }
  
  } else if (e.type == SDL_MOUSEBUTTONUP) {
    
    if (s->pressed) {
      s->pressed = false;
    }
  
  } else if (e.type == SDL_MOUSEMOTION && s->pressed) {
    
    float left  = s->slot->top_left.x;
    float right = s->slot->bottom_right.x;
    float amount = 1.0 - saturate((e.motion.x - left) / (right - left));
    
    debug("Left: %f, Right: %f, X: %f, Amount %f", left, right, (float)e.motion.x, amount);
    
    ui_slider_set_amount(s, amount);
    
  }  
  
  ui_button_event(s->label, e);
  ui_rectangle_event(s->slot, e);
  ui_rectangle_event(s->bar, e);
  
}

void ui_slider_update(ui_slider* s) {

  if (!s->active) { return; }
  
  ui_button_update(s->label);
  ui_rectangle_update(s->slot);
  ui_rectangle_update(s->bar);

}

void ui_slider_render(ui_slider* s) {

  if (!s->active) { return; }
  
  ui_button_render(s->label);
  ui_rectangle_render(s->slot);
  ui_rectangle_render(s->bar);

}