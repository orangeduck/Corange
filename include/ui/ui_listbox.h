#ifndef ui_listbox_h
#define ui_listbox_h

#include "ui/ui_rectangle.h"
#include "ui/ui_text.h"

typedef struct {
  
  ui_rectangle* back;
  
  int scroll;  
  int num_items;
  ui_text** items;
  
  bool active;
  void (*onselect)(ui_text* entry);
  
} ui_listbox;

ui_listbox* ui_listbox_new();
void ui_listbox_delete(ui_listbox* lb);

void ui_listbox_clear(ui_listbox* lb);
ui_text* ui_listbox_add_item(ui_listbox* lb, char* item);

void ui_listbox_move(ui_listbox* lb, vec2 pos);
void ui_listbox_resize(ui_listbox* lb, vec2 size);

void ui_listbox_event(ui_listbox* lb, SDL_Event e);
void ui_listbox_update(ui_listbox* lb);
void ui_listbox_render(ui_listbox* lb);

void ui_listbox_set_onselect(ui_listbox* lb, void (*onselect)(ui_text* entry));

#endif