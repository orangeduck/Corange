#include "ui/ui_listbox.h"
#include "ui/ui_style.h"

ui_listbox* ui_listbox_new() {

  ui_listbox* lb = malloc(sizeof(ui_listbox));

  lb->back = ui_rectangle_new();
  ui_rectangle_set_texture(lb->back, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(lb->back,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
  ui_rectangle_set_glitch(lb->back, ui_style_current->box_glitch);
  ui_rectangle_set_color(lb->back, ui_style_current->box_inset_color);
  
  lb->scroll = 0;
  lb->num_items = 0;
  lb->items = NULL;
  lb->active = true;
  
  lb->onselect = NULL;

  return lb;
  
}

void ui_listbox_delete(ui_listbox* lb) {
  
  ui_rectangle_delete(lb->back);
  for (int i = 0; i < lb->num_items; i++) {
    ui_text_delete(lb->items[i]);
  }
  free(lb->items);
  free(lb);
  
}

void ui_listbox_clear(ui_listbox* lb) {

  for (int i = 0; i < lb->num_items; i++) {
    ui_text_delete(lb->items[i]);
  }
  free(lb->items);
  lb->num_items = 0;
  lb->items = NULL;

}

static void move_text_items(ui_listbox* lb) {
  
  vec2 position = vec2_add(lb->back->top_left, vec2_new(5, 5));
  
  position = vec2_sub(position, vec2_new(0, 20 * lb->scroll));
  
  for (int i = 0; i < lb->num_items; i++) {
  
    ui_text_move(lb->items[i], position);
    position = vec2_add(position, vec2_new(0, 20));    
  
  }

}

ui_text* ui_listbox_add_item(ui_listbox* lb, char* item) {
  
  lb->num_items++;
  lb->items = realloc(lb->items, sizeof(ui_text*) * lb->num_items);
  
  ui_text* entry = ui_text_new();
  ui_text_draw_string(entry, item);
  ui_text_set_color(entry, ui_style_current->box_text_color);
  lb->items[lb->num_items-1] = entry;
  
  move_text_items(lb);
  return entry;
  
}

void ui_listbox_move(ui_listbox* lb, vec2 pos) {
  ui_rectangle_move(lb->back, pos);
}

void ui_listbox_resize(ui_listbox* lb, vec2 size) {
  ui_rectangle_resize(lb->back, size);
}

void ui_listbox_event(ui_listbox* lb, SDL_Event e) {
  
  if (!lb->active) { return; }
  
  if (e.type == SDL_MOUSEWHEEL && ui_rectangle_contains_point(lb->back, vec2_new(e.motion.x, e.motion.y))) {
    lb->scroll = clamp(lb->scroll+1*e.wheel.y, 0, lb->num_items); move_text_items(lb);
  }
  
  if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
  
    for (int i = 0; i < lb->num_items; i++) {
      
      ui_text* item = lb->items[i];
      ui_text_draw(item);
      
      if (ui_text_contains_point(item, vec2_new(e.motion.x, e.motion.y)) && lb->onselect) {
        debug("Selecting Item");
        lb->onselect(item);
        break;
      }
      
    }
  
  }
  
}

void ui_listbox_update(ui_listbox* lb) {

}

void ui_listbox_render(ui_listbox* lb) {
  
  ui_rectangle_render(lb->back);
  
  for(int i = lb->scroll; i < lb->num_items; i++) {
    if (ui_rectangle_contains_point(lb->back, lb->items[i]->position)) {
      ui_text_render(lb->items[i]);
    }    
  }
  
}

void ui_listbox_set_onselect(ui_listbox* lb, void (*onselect)(ui_text* entry)) {
  lb->onselect = onselect;
}
