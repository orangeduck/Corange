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

#include "cengine.h"

typedef void ui_elem;

void ui_init();
void ui_finish();

/* Pass Event, Update, and Render whole UI */
void ui_event(SDL_Event e);
void ui_update();
void ui_render();

/* Register new UI type */
#define ui_handler(type, new, delete, update, render) \
  ui_handler_cast(typeid(type), (ui_elem*(*)())new, \
                                      (void(*)(ui_elem*))delete, \
                                      (void(*)(ui_elem*))update, \
                                      (void(*)(ui_elem*))render)
                                      
void ui_handler_cast(int type_id,
  void* ui_elem_new_func(), 
  void ui_elem_del_func(void* ui_elem), 
  void ui_elem_update_func(void* ui_elem), 
  void ui_elem_render_func(void* ui_elem));


/* Create, add and get UI elements */
#define ui_elem_new(name, type) (type*)ui_elem_new_type_id(name, typeid(type))
#define ui_elem_add(name, type, ui_elem) ui_elem_add_type_id(name, typeid(type), ui_elem);
#define ui_elem_get_as(name, type) (type*)ui_elem_get_as_type_id(name, typeid(type));
#define ui_elem_add_event(name, func) ui_elem_add_event_cast(name, (void(*)(ui_elem*,SDL_Event))func)

bool ui_elem_exists(char* name);
ui_elem* ui_elem_new_type_id(char* name, int type_id);
void ui_elem_add_type_id(char* name, int type, ui_elem* ui_elem);
ui_elem* ui_elem_get(char* name);
ui_elem* ui_elem_get_as_type_id(char* name, int type_id);
void ui_elem_add_event_cast(char* name, void event_func(ui_elem* elem, SDL_Event e));

/* Update, Render, and Delete individual elements */
void ui_elem_update(char* name);
void ui_elem_render(char* name);
void ui_elem_delete(char* name);

/* Get UI element name or type name */
char* ui_elem_name(ui_elem* e);
char* ui_elem_typename(ui_elem* e);

#endif
