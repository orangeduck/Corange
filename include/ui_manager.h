#ifndef ui_manager_h
#define ui_manager_h

#include <stdbool.h>

#include "SDL/SDL.h"

#include "type.h"

typedef void ui_elem;

void ui_manager_init();
void ui_manager_finish();

void ui_event(SDL_Event e);
void ui_update();
void ui_render();

#define ui_manager_handler(type, new, delete, update, render) \
  ui_manager_handler_cast(typeid(type), (ui_elem*(*)())new, \
                                      (void(*)(ui_elem*))delete, \
                                      (void(*)(ui_elem*))update, \
                                      (void(*)(ui_elem*))render)
                                      
void ui_manager_handler_cast(int type_id, void* ui_elem_new_func(), void ui_elem_del_func(void* ui_elem), void ui_elem_update_func(void* ui_elem), void ui_elem_render_func(void* ui_elem));

bool ui_elem_exists(char* name);

#define ui_elem_new(name, type) (type*)ui_elem_new_type_id(name, typeid(type))
ui_elem* ui_elem_new_type_id(char* name, int type_id);

#define ui_elem_add(name, type, ui_elem) ui_elem_add_type_id(name, typeid(type), ui_elem);
void ui_elem_add_type_id(char* name, int type, ui_elem* ui_elem);

ui_elem* ui_elem_get(char* name);

#define ui_elem_get_as(name, type) (type*)ui_elem_get_as_type_id(name, typeid(type));
ui_elem* ui_elem_get_as_type_id(char* name, int type_id);

#define ui_elem_add_event(name, func) ui_elem_add_event_cast(name, (void(*)(ui_elem*,SDL_Event))func)
void ui_elem_add_event_cast(char* name, void event_func(ui_elem* elem, SDL_Event e));

void ui_elem_update(char* name);
void ui_elem_render(char* name);

void ui_elem_delete(char* name);

char* ui_elem_name(ui_elem* e);
char* ui_elem_typename(ui_elem* e);

#endif
