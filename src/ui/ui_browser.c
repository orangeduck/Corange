
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
  
  b->scroll = 0;
  b->num_items = 0;
  b->items = NULL;
  b->onselect = NULL;
  
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

static void move_text_items(ui_browser* b) {
  
  vec2 position = vec2_add(b->inner->top_left, vec2_new(5, 5));
  
  position = vec2_sub(position, vec2_new(0, 20 * b->scroll));
  
  for (int i = 0; i < b->num_items; i++) {
  
    ui_text_move(b->items[i], position);
    position = vec2_add(position, vec2_new(0, 20));    
  
  }

}

void ui_browser_chdir(ui_browser* b, fpath p) {
  
  for(int i = 0; i < b->num_items; i++) {
    ui_text_delete(b->items[i]);
  }
  
  debug("Setting Directory: '%s'", p.ptr);
  b->directory = p;
  b->num_items = 0;
    
  DIR* dir = opendir(p.ptr);
  if (dir == NULL) {
    error("Could not open directory '%s' to load.", p.ptr);
  }
  
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
  
  
    ui_text* entry = ui_text_new();
    ui_text_set_color(entry, vec4_light_grey());
    ui_text_draw_string(entry, ent->d_name);
    
    b->num_items++;
    b->items = realloc(b->items, sizeof(ui_text*) * b->num_items);
    b->items[b->num_items-1] = entry;
    
  }
  
  closedir(dir);
  
  move_text_items(b);
  
}

void ui_browser_event(ui_browser* b, SDL_Event e) {
  
  if (e.type == SDL_MOUSEBUTTONDOWN && ui_rectangle_contains_point(b->inner, vec2_new(e.motion.x, e.motion.y))) {
    if (e.button.button == SDL_BUTTON_WHEELUP)   { b->scroll = clamp(b->scroll-1, 0, b->num_items); move_text_items(b); }
    if (e.button.button == SDL_BUTTON_WHEELDOWN) { b->scroll = clamp(b->scroll+1, 0, b->num_items); move_text_items(b); }
  }
  
  if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
  
    for (int i = 0; i < b->num_items; i++) {
      
      ui_text* item = b->items[i];
      
      if (ui_text_contains_point(item, vec2_new(e.motion.x, e.motion.y))) {
        
        if (strcmp(item->string, ".") == 0) { continue; }
        if (strcmp(item->string, "..") == 0) {
          fpath parent;
          SDL_PathParentDirectory(parent.ptr, b->directory.ptr);
          ui_browser_chdir(b, parent);
          continue;
        }
        
        fpath child;
        strcpy(child.ptr, b->directory.ptr);
        strcat(child.ptr, "/");
        strcat(child.ptr, item->string);
        
        if (SDL_PathIsDirectory(child.ptr)) {
          ui_browser_chdir(b, child);
          continue;
        }
        
        if (SDL_PathIsFile(child.ptr)) {
          if (b->onselect) {
            b->onselect(child);
          }
          continue;
        }
        
      }
      
    }
  
  }
  
}

void ui_browser_set_onselect(ui_browser* b, void (*onselect)(fpath)) {
  b->onselect = onselect;
}

void ui_browser_update(ui_browser* b) {

}

void ui_browser_render(ui_browser* b) {
  
  ui_rectangle_render(b->outer);
  ui_rectangle_render(b->inner);
  
  for(int i = b->scroll; i < b->num_items; i++) {
    if (ui_rectangle_contains_point(b->inner, b->items[i]->position)) {
      ui_text_render(b->items[i]);
    }    
  }
  
  
}
