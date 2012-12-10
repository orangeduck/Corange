
#include "ui/ui_browser.h"

#include "cgraphics.h"

ui_browser* ui_browser_new() {
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  ui_browser* b = malloc(sizeof(ui_browser));
  
  b->outer = ui_rectangle_new();
  ui_rectangle_move(b->outer, vec2_new(width - 300, 10));
  ui_rectangle_resize(b->outer, vec2_new(280, height - 20));
  ui_rectangle_set_texture(b->outer, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(b->outer, 1, vec4_black());
  ui_rectangle_set_color(b->outer, vec4_grey());
  ui_rectangle_set_glitch(b->outer, 1.0);
  
  b->inner = ui_rectangle_new();
  ui_rectangle_move(b->inner, vec2_new(width - 290, 20));
  ui_rectangle_resize(b->inner, vec2_new(260, height - 40));
  ui_rectangle_set_texture(b->inner, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(b->inner, 1, vec4_black());
  ui_rectangle_set_color(b->inner, vec4_new(0.1, 0.1, 0.1, 1));
  ui_rectangle_set_glitch(b->inner, 1.0);
  
  strcpy(b->directory.ptr, SDL_GetWorkingDir());
  
  b->num_items = 0;
  b->items = NULL;
  
  ui_browser_chdir(b, b->directory);
  
  return b;
  
}

void ui_browser_delete(ui_browser* b) {
  
  ui_rectangle_delete(b->outer);
  ui_rectangle_delete(b->inner);
  
  for(int i = 0; i < b->num_items; i++) {
    ui_text_delete(b->items[i]);
  }
  
  free(b->items);
  free(b);
  
}

void ui_browser_chdir(ui_browser* b, fpath p) {
  
  for(int i = 0; i < b->num_items; i++) {
    ui_text_delete(b->items[i]);
  }
  
  b->num_items = 0;
  
  vec2 position = vec2_add(b->inner->top_left, vec2_new(5, 5));
  
  DIR* dir = opendir(p.ptr);
  if (dir == NULL) {
    error("Could not open directory '%s' to load.", p.ptr);
  }
  
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
  
    position = vec2_add(position, vec2_new(0, 20));
  
    ui_text* entry = ui_text_new();
    ui_text_move(entry, position);
    ui_text_set_color(entry, vec4_light_grey());
    ui_text_draw_string(entry, ent->d_name);
    
    b->num_items++;
    b->items = realloc(b->items, sizeof(ui_text*) * b->num_items);
    b->items[b->num_items-1] = entry;
    
  }
  
  closedir(dir);
  
}

void ui_browser_event(ui_browser* b, SDL_Event e) {
  
  if (e.type == SDL_MOUSEBUTTONUP) {
  
    for (int i = 0; i < b->num_items; i++) {
      
      ui_text* item = b->item[i];
      
      if (ui_text_contains_point(item, vec2_new(e.motion.x, e.motion.y)) {
        
        if (strcmp(item->string, ".") == 0) {}
        if (strcmp(item->string, "..") == 0) {
          
        }
        
      }
      
    }
  
  }
  
}

void ui_browser_update(ui_browser* b) {

}

void ui_browser_render(ui_browser* b) {
  
  ui_rectangle_render(b->outer);
  ui_rectangle_render(b->inner);
  
  for(int i = 0; i < b->num_items; i++) {
    ui_text_render(b->items[i]);
  }
  
  
}
