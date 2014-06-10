
#include "ui/ui_toast.h"

#include "cui.h"
#include "cgraphics.h"

#define MAX_TOASTS 128
static int toasts_num = 0;
static ui_toast* toasts[MAX_TOASTS];

static void reposition_toasts() {
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  float start = 2 * (height / 3);
  
  for (int i = 0; i < toasts_num; i++) {
    ui_text_move(toasts[i]->label, vec2_new(20, start + i * 20));
  }
  
}

static void shift_toasts_down(ui_toast* new) {

  if (toasts_num == MAX_TOASTS) {
    warning("Too Many Toasts!");
    return;
  }
  
  memmove(&toasts[1], &toasts[0], sizeof(ui_toast*) * toasts_num);
  toasts[0] = new;
  toasts_num++;
  
  reposition_toasts();
  
}

static void shift_toasts_up() {

  toasts_num--;
  
  reposition_toasts();
  
}

static int toast_index(ui_toast* t) {
  
  for (int i = 0; i < toasts_num; i++) {
    if (toasts[i] == t) return i;
  }
  
  return -1;
  
}

static unsigned int popup_counter = 0;

static char popup_contents[1024];

void ui_toast_popup(char* fmt, ...) {
  
  va_list va;
  va_start(va, fmt);
  vsnprintf(popup_contents, 1023, fmt, va);
  va_end(va);

  ui_toast* t = ui_elem_new("toast_%i", ui_toast, popup_counter++);
  ui_toast_set_label(t, popup_contents);
  
}

ui_toast* ui_toast_new() {
  
  ui_toast* t = malloc(sizeof(ui_toast));
  
  t->label = ui_text_new();
  ui_text_move(t->label, vec2_new(0,0));
  ui_text_set_color(t->label, vec4_light_grey());
  ui_text_set_scale(t->label, vec2_new(1.25, 1.25));
  ui_text_align(t->label, TEXT_ALIGN_LEFT, TEXT_ALIGN_LEFT);
  ui_text_draw_string(t->label, "Toast!");
  
  t->opacity = 1.0;
  t->active = true;
  
  shift_toasts_down(t);
  
  return t;
  
}

void ui_toast_delete(ui_toast* t) {
  
  shift_toasts_up();
  
  ui_text_delete(t->label);
  free(t);
  
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
  
  t->opacity -= frame_time() * 0.1;
  
  if (t->opacity < 0 || toast_index(t) > 50) {
    ui_elem_delete(ui_elem_name(t));
  }
  
}

void ui_toast_render(ui_toast* t) {
  
  vec4 lcol = t->label->color;
  ui_text_set_color(t->label, vec4_new(lcol.x, lcol.y, lcol.z, t->opacity));
  ui_text_render(t->label);

}
