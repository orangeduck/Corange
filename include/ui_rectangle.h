#ifndef ui_rectangle_h
#define ui_rectangle_h

#include "texture.h"

typedef struct ui_rectangle_temp {

  vector2 top_left;
  vector2 bottom_right;
  
  vector4 color;
  texture* texture;
  
  float border_size;
  vector4 border_color;
  
  void (*on_click_up_listener)(struct ui_rectangle_temp*);
  void (*on_click_down_listener)(struct ui_rectangle_temp*);
  void (*on_mouse_over_listener)(struct ui_rectangle_temp*);
  void (*on_mouse_move_listener)(struct ui_rectangle_temp*);
  
} ui_rectangle;

ui_rectangle* ui_rectangle_new(vector2 top_left, vector2 bottom_right);
void ui_rectangle_delete(ui_rectangle* rect);

void ui_rectangle_set_texture(ui_rectangle* rect, texture* t);

void ui_rectangle_render(ui_rectangle* rect);

int ui_rectangle_contains_position(ui_rectangle* text, vector2 position);

void ui_rectangle_set_on_click_down_listener(ui_rectangle* text, void (*listener)(ui_rectangle*));
void ui_rectangle_set_on_click_up_listener(ui_rectangle* text, void (*listener)(ui_rectangle*));
void ui_rectangle_set_on_mouse_over_listener(ui_rectangle* text, void (*listener)(ui_rectangle*));
void ui_rectangle_set_on_mouse_move_listener(ui_rectangle* text, void (*listener)(ui_rectangle*));

int ui_rectangle_on_click_up(ui_rectangle* text, vector2 pos);
int ui_rectangle_on_click_down(ui_rectangle* text, vector2 pos);
int ui_rectangle_on_mouse_over(ui_rectangle* text, vector2 pos);
int ui_rectangle_on_mouse_move(ui_rectangle* text, vector2 pos);

#endif