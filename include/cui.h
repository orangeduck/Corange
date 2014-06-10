/**
*** :: UI ::
***
***   Interface to create and manager UI data.
***   
***   Acts much like the entity manager.
***   UI elements can be created, destroyed and
***   accessed from anywhere in the engine.
***
**/

#ifndef cui_h
#define cui_h

#include "ui/ui_style.h"

#include "cengine.h"

typedef void ui_elem;

void ui_init(void);
void ui_finish(void);
void ui_set_style(ui_style* s);

/* Pass Event, Update, and Render whole UI */
void ui_event(SDL_Event e);
void ui_update(void);
void ui_render(void);

/* Register new UI type */
#define ui_handler(type, new, delete, event, update, render) \
  ui_handler_cast(typeid(type), \
  (ui_elem*(*)())new, \
  (void(*)(ui_elem*))delete, \
  (void(*)(ui_elem*,SDL_Event))event, \
  (void(*)(ui_elem*))update, \
  (void(*)(ui_elem*))render)

void ui_handler_cast(int type_id,
  void* (*ui_elem_new_func)(), 
  void (*ui_elem_del_func)(ui_elem*), 
  void (*ui_elem_event_func)(ui_elem*, SDL_Event), 
  void (*ui_elem_update_func)(ui_elem*), 
  void (*ui_elem_render_func)(ui_elem*));

/* Create, add and get UI elements */
#define ui_elem_new(fmt, type, ...) (type*)ui_elem_new_type_id(fmt, typeid(type), ##__VA_ARGS__)
#define ui_elem_get_as(fmt, type, ...) ((type*)ui_elem_get_as_type_id(fmt, typeid(type), ##__VA_ARGS__));

bool ui_elem_exists(char* fmt, ...);
ui_elem* ui_elem_get(char* fmt, ...);
ui_elem* ui_elem_get_as_type_id(char* fmt, int type_id, ...);
ui_elem* ui_elem_new_type_id(char* fmt, int type_id, ...);
void ui_elem_delete(char* fmt, ...);
void ui_elem_event(char* fmt, SDL_Event e, ...);
void ui_elem_update(char* fmt, ...);
void ui_elem_render(char* fmt, ...);

/* Get UI element name or type name */
char* ui_elem_name(ui_elem* e);
char* ui_elem_typename(ui_elem* e);

#endif
