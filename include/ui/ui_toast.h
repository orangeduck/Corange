/**
*** :: UI Toast ::
***
***   UI Notification
***
**/

#ifndef ui_toast_h
#define ui_toast_h

#include "cengine.h"
#include "ui/ui_text.h"
#include "ui/ui_rectangle.h"

typedef struct {

  ui_rectangle* back;
  ui_text* label;
  
  float opacity;
  
  bool active;
} ui_toast;

ui_toast* ui_toast_new();
void ui_toast_delete(ui_toast* t);

void ui_toast_resize(ui_toast* t, vec2 size);
void ui_toast_set_label(ui_toast* t, char* label);
void ui_toast_set_font(ui_toast* t, asset_hndl f);

void ui_toast_event(ui_toast* t, SDL_Event e);
void ui_toast_update(ui_toast* t);
void ui_toast_render(ui_toast* t);

#endif
