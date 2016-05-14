#include "ui/ui_dialog.h"

#include "cgraphics.h"

int dialog_count = 0;

ui_dialog* ui_dialog_new() {
  
  ui_dialog* d = malloc(sizeof(ui_dialog));
  
  int width  = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  d->back = ui_button_new();
  ui_button_move(d->back, vec2_new(width/2 - 200, height/2 - 100));
  ui_button_resize(d->back, vec2_new(400, 200));
  ui_button_set_label(d->back, "");
  ui_button_disable(d->back);
  
  d->title = ui_text_new();
  ui_text_move(d->title, vec2_new(width/2, height/2 - 50));
  ui_text_align(d->title, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
  ui_text_set_scale(d->title, vec2_new(1.25, 1.25));
  ui_text_set_color(d->title, vec4_light_grey());
  ui_text_draw_string(d->title, "Title");
  
  d->contents = ui_text_new();
  ui_text_move(d->contents, vec2_new(width/2, height/2 - 00));
  ui_text_align(d->contents, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
  ui_text_set_color(d->contents, vec4_light_grey());
  ui_text_draw_string(d->contents, "Contents");
  
  d->left = ui_button_new();
  ui_button_move(d->left, vec2_new(width/2 - 175, height/2 + 40));
  ui_button_resize(d->left, vec2_new(150, 40));
  ui_button_set_label(d->left, "Left");
  
  d->right = ui_button_new();
  ui_button_move(d->right, vec2_new(width/2 + 55, height/2 + 40)); 
  ui_button_resize(d->right, vec2_new(150, 40));
  ui_button_set_label(d->right, "Right");
  
  dialog_count++;
  
  return d;
  
}

void ui_dialog_set_font(ui_dialog* d, asset_hndl fnt) {
  ui_text_set_font(d->title, fnt);
  ui_text_set_font(d->contents, fnt);
  ui_button_set_font(d->left, fnt);
  ui_button_set_font(d->right, fnt);
}

void ui_dialog_set_single_button(ui_dialog* d, bool single) { 

  int width  = graphics_viewport_width();
  int height = graphics_viewport_height();

  d->single_button = single;

  if (single) {
    ui_button_move(d->left, vec2_new(width/2 - 75, height/2 + 40));
    ui_button_set_active(d->right, false);
  } else {
    ui_button_move(d->left, vec2_new(width/2 - 175, height/2 + 40));
    ui_button_set_active(d->right, true);
  }

}

void ui_dialog_delete(ui_dialog* d) {
  
  dialog_count--;
  
  ui_button_delete(d->back);
  ui_text_delete(d->title);
  ui_text_delete(d->contents);
  ui_button_delete(d->left);
  ui_button_delete(d->right);
  
  free(d);
  
}

void ui_dialog_set_title(ui_dialog* d, char* title) {
  ui_text_draw_string(d->title, title);
}

void ui_dialog_set_contents(ui_dialog* d, char* contents) {
  ui_text_draw_string(d->contents, contents);
}

void ui_dialog_set_button_left(ui_dialog* d, char* left, void (*onleft)(ui_button*, void*)) {
  ui_button_set_label(d->left, left);
  ui_button_set_onclick(d->left, onleft);
}

void ui_dialog_set_button_right(ui_dialog* d, char* right, void (*onright)(ui_button*, void*)) {
  ui_button_set_label(d->right, right);
  ui_button_set_onclick(d->right, onright);
}

void ui_dialog_event(ui_dialog* d, SDL_Event e) {

  ui_button_event(d->back, e);
  ui_text_event(d->title, e);
  ui_text_event(d->contents, e);
  ui_button_event(d->left, e);
  ui_button_event(d->right, e);

}

void ui_dialog_update(ui_dialog* d) {

  ui_button_update(d->back);
  ui_text_update(d->title);
  ui_text_update(d->contents);
  ui_button_update(d->left);
  ui_button_update(d->right);

}

void ui_dialog_render(ui_dialog* d) {
  
  ui_button_render(d->back);
  ui_text_render(d->title);
  ui_text_render(d->contents);
  ui_button_render(d->left);
  ui_button_render(d->right);
  
}