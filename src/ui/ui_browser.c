
#include "ui/ui_browser.h"

#include "cgraphics.h"

ui_browser* ui_browser_new() {
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  ui_browser* b = malloc(sizeof(ui_browser));
  
  b->outer = ui_rectangle_new();
  ui_rectangle_move(b->outer, vec2_new(width - 300, 10));
  ui_rectangle_resize(b->outer, vec2_new(280, height - 20));
  ui_rectangle_set_texture(b->outer, asset_hndl_new_load(P("$CORANGE/ui/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(b->outer, 1, vec4_black());
  ui_rectangle_set_color(b->outer, vec4_grey());
  ui_rectangle_set_glitch(b->outer, 1.0);
  
  b->inner = ui_listbox_new();
  ui_listbox_move(b->inner, vec2_new(width - 290, 20));
  ui_listbox_resize(b->inner, vec2_new(260, height - 40));
  
  strcpy(b->directory.ptr, SDL_GetWorkingDir());
  ui_browser_chdir(b, b->directory);
  
  b->active = true;
  
  return b;
  
}

void ui_browser_delete(ui_browser* b) {
  
  ui_rectangle_delete(b->outer);
  ui_listbox_delete(b->inner);
  
  free(b);
  
}

void ui_browser_chdir(ui_browser* b, fpath p) {
  
  
  b->directory = p;
  debug("Setting Directory: '%s'", p.ptr);
  
  DIR* dir = opendir(p.ptr);
  if (dir == NULL) {
    error("Could not open directory '%s' to load.", p.ptr);
  }
  
  ui_listbox_clear(b->inner);

  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
  
    ui_listbox_add_item(b->inner, ent->d_name);
    
  }
  
  closedir(dir);
  
}

void ui_browser_event(ui_browser* b, SDL_Event e) {
  
  if (!b->active) { return; }
  
  ui_listbox_event(b->inner, e);
  
}

void ui_browser_update(ui_browser* b) {

}

void ui_browser_render(ui_browser* b) {
  
  if (!b->active) { return; }
  
  ui_rectangle_render(b->outer);
  ui_listbox_render(b->inner);
  
}
