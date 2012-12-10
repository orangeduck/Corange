#ifndef ui_browser_h
#define ui_browser_h

#include "ui/ui_rectangle.h"
#include "ui/ui_text.h"

typedef struct {
  
  ui_rectangle* outer;
  ui_rectangle* inner;
  
  fpath directory;
  
  int num_items;
  ui_text** items;
  
  void (*onselect)(fpath file);

} ui_browser;

ui_browser* ui_browser_new();
void ui_browser_delete(ui_browser* b);

void ui_browser_chdir(ui_browser* b, fpath dir);

void ui_browser_event(ui_browser* b, SDL_Event e);
void ui_browser_update(ui_browser* b);
void ui_browser_render(ui_browser* b);

#endif