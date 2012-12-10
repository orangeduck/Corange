#include "ui/ui_button.h"

#include "ui/ui_rectangle.h"

#include "cgraphics.h"

ui_button* ui_button_new() {

  ui_button* b = malloc(sizeof(ui_button));
  
  b->back = ui_rectangle_new();
  ui_rectangle_set_texture(b->back, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(b->back, 1, vec4_black());
  ui_rectangle_set_glitch(b->back, 1.0);
  
  b->label = ui_text_new_string("Button1");
  ui_text_move(b->label, ui_rectangle_center(b->back));
  ui_text_set_color(b->label, vec4_light_grey());
  ui_text_align(b->label, text_align_center, text_align_center);
  ui_text_draw(b->label);
  
  b->onclick = NULL;
  
  b->up_color = vec4_new(0.1, 0.1, 0.1, 1);
  b->down_color = vec4_grey();

  b->active = true;
  b->enabled = true;
  b->pressed = false;
  
  return b;

}

void ui_button_delete(ui_button* b) {
  
  ui_rectangle_delete(b->back);
  ui_text_delete(b->label);
  free(b);
  
}

void ui_button_event(ui_button* b, SDL_Event e) {
  
  if (e.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_button_contains_point(b, vec2_new(e.motion.x, e.motion.y))) {
      b->pressed = true;
    }
  
  } else if (e.type == SDL_MOUSEBUTTONUP) {
    
    if (b->pressed) {
      b->pressed = false;
      if (b->onclick) {
        b->onclick();
      }
    }
  }
  
}

void ui_button_update(ui_button* b) {
  
  ui_rectangle_update(b->back);
  ui_text_update(b->label);
  
}

void ui_button_move(ui_button* b, vec2 pos) {
  ui_rectangle_move(b->back, pos);
  ui_text_move(b->label, ui_rectangle_center(b->back));
}

void ui_button_resize(ui_button* b, vec2 size) {
  ui_rectangle_resize(b->back, size);
  ui_text_move(b->label, ui_rectangle_center(b->back));
}

void ui_button_set_font(ui_button* b, asset_hndl f) {
  ui_text_set_font(b->label, f);
}

void ui_button_set_label(ui_button* b, char* label) {
  ui_text_draw_string(b->label, label);
}

void ui_button_set_onclick(ui_button* b, void(*onclick)(void)) {
  b->onclick = onclick;
}

void ui_button_render(ui_button* b) {
  
  if (b->pressed || !b->enabled) {
    ui_rectangle_set_color(b->back, b->down_color);
  } else {
    ui_rectangle_set_color(b->back, b->up_color);
  }
  
  ui_rectangle_render(b->back);
  ui_text_render(b->label);
  
}

void ui_button_disable(ui_button* b) {
  b->enabled = false;
}

void ui_button_enable(ui_button* b) {
  b->enabled = true;
}

bool ui_button_contains_point(ui_button* b, vec2 pos) {

  if (!b->active) { return false; }
  
  return ui_rectangle_contains_point(b->back, pos);

}
