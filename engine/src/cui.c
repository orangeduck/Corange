#include "cui.h"

#include "data/dict.h"
#include "data/list.h"

typedef struct {

  int type_id;
  void* (*new_func)();
  void (*del_func)(ui_elem*);
  void (*update_func)(ui_elem*);
  void (*render_func)(ui_elem*);

} ui_elem_handler;

#define MAX_UI_ELEM_HANDLERS 512
static ui_elem_handler ui_elem_handlers[MAX_UI_ELEM_HANDLERS];
static int num_ui_elem_handlers = 0;


typedef struct {
  char* name;
  void (*event_func)(ui_elem*,SDL_Event);
} ui_event_handler;

#define MAX_UI_EVENTS 512
static ui_event_handler ui_events[MAX_UI_EVENTS];
static int num_ui_events = 0;


static list* ui_elem_names;
static dict* ui_elems;
static dict* ui_elem_types;

void ui_init() {
  ui_elems = dict_new(512);
  ui_elem_types = dict_new(512);
  
  ui_elem_names = list_new(512);
}

void ui_finish() {

  for(int i = 0; i < num_ui_events; i++) {
    free(ui_events[i].name);
  }

  for(int i = 0; i < ui_elem_names->num_items; i++) {
    char* name = list_get(ui_elem_names, i);
    int* type_id = dict_get(ui_elem_types, name);
    debug("Deleting UI Element %s (%s)", name, type_id_name(*type_id));
    ui_elem_delete(name);
  }
  
  list_delete_with(ui_elem_names, free);
  
  dict_delete(ui_elems);
  
  dict_map(ui_elem_types, free);
  dict_delete(ui_elem_types);

}

void ui_event(SDL_Event e) {
  
  for(int i = 0; i < num_ui_events; i++) {
    char* elem_name = ui_events[i].name;
    ui_events[i].event_func( ui_elem_get(elem_name) , e);
  }  

}

void ui_update() {

  for (int i = 0; i < ui_elem_names->num_items; i++) {
    char* name = list_get(ui_elem_names, i);
    int* type_id = dict_get(ui_elem_types, name);
    ui_elem_update(name);
  }

}

void ui_render() {

  for(int i = 0; i < ui_elem_names->num_items; i++) {
    char* name = list_get(ui_elem_names, i);
    int* type_id = dict_get(ui_elem_types, name);
    ui_elem_render(name);
  }

}

void ui_handler_cast(int type_id, void* ui_elem_new_func(), void ui_elem_del_func(void* ui_elem), void ui_elem_update_func(void* ui_elem), void ui_elem_render_func(void* ui_elem)) {
  
  if (num_ui_elem_handlers >= MAX_UI_ELEM_HANDLERS) {
    warning("Max number of ui element handlers reached. Handler for type %s not added.", type_id_name(type_id));
    return;
  }
  
  ui_elem_handler ui_hand;
  ui_hand.type_id = type_id;
  ui_hand.new_func = ui_elem_new_func;
  ui_hand.del_func = ui_elem_del_func;
  ui_hand.update_func = ui_elem_update_func;
  ui_hand.render_func = ui_elem_render_func;
  
  ui_elem_handlers[num_ui_elem_handlers] = ui_hand;
  num_ui_elem_handlers++; 
  
}

bool ui_elem_exists(char* name) {
  if (dict_get(ui_elems, name)) {
    return true;
  } else {
    return false;
  }
}

ui_elem* ui_elem_new_type_id(char* name, int type_id) {
  
  if ( dict_contains(ui_elems, name) ) {
    error("UI Manager already contains element called %s!", name);
  }
  
  debug("Creating UI Element %s (%s)", name, type_id_name(type_id));
  
  ui_elem* ui_e = NULL;
  
  for(int i = 0; i < num_ui_elem_handlers; i++) {
    ui_elem_handler ui_hand = ui_elem_handlers[i];
    if (ui_hand.type_id == type_id) {
      ui_e = ui_hand.new_func();
    }
  }
  
  if (ui_e == NULL) {
    error("Don't know how to create ui element %s. No handler for type %s!", name, type_id_name(type_id));
  }
  
  dict_set(ui_elems, name, ui_e);
  
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type_id;
  dict_set(ui_elem_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(ui_elem_names, name_copy);
  
  return ui_e;
  
}

void ui_elem_add_type_id(char* name, int type_id, ui_elem* ui_elem) {

  if ( dict_contains(ui_elems, name) ) {
    error("UI Manager already contains element called %s!", name);
  }
  
  dict_set(ui_elems, name, ui_elem);
  
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type_id;
  dict_set(ui_elem_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(ui_elem_names, name_copy);

}

ui_elem* ui_elem_get(char* name) {

  if ( !ui_elem_exists(name) ) {
    error("UI element %s does not exist!", name);
  }
  
  return dict_get(ui_elems, name);

}

ui_elem* ui_elem_get_as_type_id(char* name, int type_id) {

  if ( !ui_elem_exists(name) ) {
    error("UI element %s does not exist!", name);
  }
  
  int* ui_elem_type = dict_get(ui_elem_types, name);
  
  if (*ui_elem_type != type_id) {
    error("UI element %s was created/added as a %s, but you requested it as a %s!", name, type_id_name(*ui_elem_type), type_id_name(type_id));
  }
  
  return dict_get(ui_elems, name);

}

void ui_elem_add_event_cast(char* name, void event_func(ui_elem* elem, SDL_Event e)) {
  
  if (num_ui_events >= MAX_UI_EVENTS) {
    warning("Already reached maximum number of %i ui event handlers. Handler for ui element %s not added", MAX_UI_EVENTS, name);
    return;
  }
  
  ui_event_handler ui_eh;
  ui_eh.name = malloc(strlen(name) + 1);
  strcpy(ui_eh.name, name);
  ui_eh.event_func = event_func;
  
  ui_events[num_ui_events] = ui_eh;
  num_ui_events++;
  
}

void ui_elem_update(char* name) {

  ui_elem* elem = ui_elem_get(name);
  int* type_ptr = dict_get(ui_elem_types, name);
  int type_id = *type_ptr;

  for(int i = 0; i < num_ui_elem_handlers; i++) {
    ui_elem_handler ui_hand = ui_elem_handlers[i];
    if (ui_hand.type_id == type_id) {
      ui_hand.update_func(elem);
      break;
    }
  }

}

void ui_elem_render(char* name) {

  ui_elem* elem = ui_elem_get(name);
  int* type_ptr = dict_get(ui_elem_types, name);
  int type_id = *type_ptr;

  for(int i = 0; i < num_ui_elem_handlers; i++) {
    ui_elem_handler ui_hand = ui_elem_handlers[i];
    if (ui_hand.type_id == type_id) {
      ui_hand.render_func(elem);
      break;
    }
  }

}

void ui_elem_delete(char* name) {

  int* type_ptr = dict_get(ui_elem_types, name);
  int type_id = *type_ptr;

  for(int i = 0; i < num_ui_elem_handlers; i++) {
    ui_elem_handler ui_hand = ui_elem_handlers[i];
    if (ui_hand.type_id == type_id) {
      dict_remove_with(ui_elems, name, ui_hand.del_func);
      break;
    }
  }
  
  if (ui_elem_exists(name)) {
    error("Don't know how to delete UI element %s. No handler for type %s!", name, type_id_name(type_id));
  }

}

char* ui_elem_name(ui_elem* e) {
  
  for(int i = 0; i < ui_elem_names->num_items; i++) {
    char* name = list_get(ui_elem_names, i);
    ui_elem* elem = dict_get(ui_elems, name);
    
    if (elem == e) {
      return name;
    }
  }
  
  error("UI Object at %p not loaded into ui manager. Cannot fetch name.", e);
  return NULL;
}

char* ui_elem_typename(ui_elem* e) {
  
  for(int i = 0; i < ui_elem_names->num_items; i++) {
    char* name = list_get(ui_elem_names, i);
    ui_elem* elem = dict_get(ui_elems, name);
    
    if (elem == e) {
      int* type = dict_get(ui_elem_types, name);
      return type_id_name(*type);
    }
  }
  
  error("UI Object at %p not loaded into ui manager. Cannot fetch type name.", e);
  return NULL;
}



