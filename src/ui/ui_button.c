#include "ui/ui_button.h"
#include "ui/ui_style.h"
#include "ui/ui_rectangle.h"

#include "cgraphics.h"

ui_button* ui_button_new() {

  ui_button* b = malloc(sizeof(ui_button));
  
  b->back = ui_rectangle_new();
  ui_rectangle_set_texture(b->back, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(b->back,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
  ui_rectangle_set_glitch(b->back, ui_style_current->box_glitch);
  ui_rectangle_blend(b->back, 
    ui_style_current->box_blend_src,
    ui_style_current->box_blend_dst);
  
  b->label = ui_text_new_string("Button1");
  ui_text_move(b->label, ui_rectangle_center(b->back));
  ui_text_set_color(b->label, ui_style_current->box_text_color);
  ui_text_align(b->label, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
  ui_text_draw(b->label);
  
  b->onclick = NULL;
  b->onclick_data = NULL;
  
  b->up_color = ui_style_current->box_up_color;
  b->down_color = ui_style_current->box_down_color;

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
        b->onclick(b, b->onclick_data);
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

void ui_button_set_label_color(ui_button* b, vec4 color) {
  b->label->color = color;
  ui_text_draw(b->label);
}

void ui_button_set_onclick(ui_button* b, void(*onclick)(ui_button*, void*)) {
  b->onclick = onclick;
}

void ui_button_set_onclick_data(ui_button* b, void* data) {
  b->onclick_data = data;
}

void ui_button_set_active(ui_button* b, bool active) {
  b->active = active;
}

void ui_button_set_enabled(ui_button* b, bool enabled) {
  b->enabled = enabled;
}

void ui_button_set_texture(ui_button* b, asset_hndl tex, int width, int height, bool tile) {
  ui_rectangle_set_texture(b->back, tex, width, height, tile);
}


vec2 ui_button_position(ui_button* b) {
  return ui_rectangle_position(b->back);
}

vec2 ui_button_size(ui_button* b) {
  return ui_rectangle_size(b->back);
}

void ui_button_render(ui_button* b) {
  
  if (!b->active) { return; }
  
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
