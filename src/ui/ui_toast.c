
#include "ui/ui_toast.h"

#include "cui.h"
#include "cgraphics.h"

ui_toast* ui_toast_new() {
  
  ui_toast* t = malloc(sizeof(ui_toast));
  
  t->back = ui_rectangle_new();
  ui_rectangle_resize(t->back, vec2_new(150, 30));
  ui_rectangle_set_texture(t->back, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(t->back, 1, vec4_black());
  ui_rectangle_set_glitch(t->back, 1.0);
  
  t->label = ui_text_new();
  ui_text_move(t->label, ui_rectangle_center(t->back));
  ui_text_set_color(t->label, vec4_light_grey());
  ui_text_align(t->label, text_align_center, text_align_center);
  ui_text_draw_string(t->label, "Toast!");
  
  t->opacity = 1.0;
  t->active = true;
  
  return t;
  
}

void ui_toast_delete(ui_toast* t) {
  
  ui_rectangle_delete(t->back);
  ui_text_delete(t->label);
  
  free(t);
  
}

void ui_toast_resize(ui_toast* t, vec2 size) {
  ui_rectangle_resize(t->back, size);
  ui_text_move(t->label, ui_rectangle_center(t->back));
}

void ui_toast_set_label(ui_toast* t, char* label) {
  ui_text_draw_string(t->label, label);

}

void ui_toast_set_font(ui_toast* t, asset_hndl f) {
  ui_text_set_font(t->label, f);
}

void ui_toast_event(ui_toast* t, SDL_Event e) {
}

void ui_toast_update(ui_toast* t) {
  
  t->opacity -= frame_time() * 0.5;
  
  if (t->opacity < 0) {
    ui_elem_delete(ui_elem_name(t));
  }
  
}

void ui_toast_render(ui_toast* t) {

  int width = graphics_viewport_width();
  int height = graphics_viewport_height();

  vec2 position = vec2_new(width / 2 - 75, height / 2 - 15);
  ui_rectangle_move(t->back, position);
  ui_text_move(t->label, ui_rectangle_center(t->back));
  
  vec4 back_col = t->back->color;
  float back_border_size = t->back->border_size;
  vec4 back_border_color = t->back->border_color;
  ui_rectangle_set_color(t->back, vec4_new(back_col.x, back_col.y, back_col.z, t->opacity));
  ui_rectangle_set_border(t->back, back_border_size, vec4_new(back_border_color.x, back_border_color.y, back_border_color.z, t->opacity));
  
  vec4 lab_col = t->label->color;
  ui_text_set_color(t->label, vec4_new(lab_col.x, lab_col.y, lab_col.z, t->opacity));
  
  ui_rectangle_render(t->back);
  ui_text_render(t->label);

}
