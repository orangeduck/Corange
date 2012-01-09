#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "dictionary.h"
#include "list.h"

#include "entity_manager.h"

typedef struct {

  int type_id;
  void* (*new_func)();
  void (*del_func)();

} entity_handler;

#define MAX_ENTITY_HANDLERS 512
static entity_handler entity_handlers[MAX_ENTITY_HANDLERS];
static int num_entity_handlers = 0;

static list* entity_names;
static dictionary* entities;
static dictionary* entity_types;

void entity_manager_init() {
  
  entities = dictionary_new(512);
  entity_types = dictionary_new(512);
  
  entity_names = list_new(512);
}

void entity_manager_finish() {
    
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type_id = dictionary_get(entity_types, name);
    entity_delete(name);
  }
  
  list_delete_with(entity_names, free);
  
  dictionary_delete(entities);
  
  dictionary_map(entity_types, free);
  dictionary_delete(entity_types);
  
}

void entity_manager_handler_cast(int type_id, void* entity_new_func() , void entity_del_func(void* entity)) {
  
  if (num_entity_handlers >= MAX_ENTITY_HANDLERS) {
    warning("Max number of entity handlers reached. Handler for type %s not added.", type_id_name(type_id));
    return;
  }
  
  entity_handler eh;
  eh.type_id = type_id;
  eh.new_func = entity_new_func;
  eh.del_func = entity_del_func;
  
  entity_handlers[num_entity_handlers] = eh;
  num_entity_handlers++;
}

bool entity_exists(char* name) {
  return dictionary_contains(entities, name);
}

entity* entity_new_type_id(char* name, int type_id) {

  if ( dictionary_contains(entities, name) ) {
    error("Entity Manager already contains entity called %s!", name);
  }
  
  debug("Creating Entity %s (%s)", name, type_id_name(type_id));
  
  entity* e = NULL;
  
  int i;
  for(i = 0; i < num_entity_handlers; i++) {
    entity_handler eh = entity_handlers[i];
    if (eh.type_id == type_id) {
      e = eh.new_func();
    }
  }
  
  if (e == NULL) {
    error("Don't know how to create entity %s. No handler for type %s!", name, type_id_name(type_id));
  }
  
  dictionary_set(entities, name, e);
  
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type_id;
  dictionary_set(entity_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(entity_names, name_copy);
  
  return e;
}

void entity_add_type_id(char* name, int type_id, entity* entity) {

  if ( entity_exists(name) ) {
    error("Entity Manager already contains entity called %s!", name);
  }
  
  dictionary_set(entities, name, entity);
  
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type_id;
  dictionary_set(entity_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(entity_names, name_copy);
}

entity* entity_get(char* name) {
  
  if ( !entity_exists(name) ) {
    error("Entity %s does not exist!", name);
  }
  
  return dictionary_get(entities, name);
  
}

entity* entity_get_as_type_id(char* name, int type_id) {
  
  if ( !entity_exists(name) ) {
    error("Entity %s does not exist!", name);
  }
  
  int* entity_type = dictionary_get(entity_types, name);
  
  if (*entity_type != type_id) {
    error("Entity %s was created/added as a %s, but you requested it as a %s!", name, type_id_name(*entity_type), type_id_name(type_id));
  }
  
  return dictionary_get(entities, name);
}

int entity_type_count_type_id(int type_id) {
  
  int count = 0;
  
  int i;
  for(i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type = dictionary_get(entity_types, name);
    
    if (*type == type_id) {
      count++;
    }
  }
  
  return count;
  
}

void entity_delete(char* name) {
  
  int* type_ptr = dictionary_get(entity_types, name);
  int type_id = *type_ptr;
  
  debug("Deleting Entity %s (%s)", name, type_id_name(type_id));
  
  int i;
  for(i = 0; i < num_entity_handlers; i++) {
    entity_handler eh = entity_handlers[i];
    if (eh.type_id == type_id) {
      dictionary_remove_with(entities, name, eh.del_func);
      break;
    }
  }
  
  for(i = 0; i < entity_names->num_items; i++) {
    if ( strcmp(list_get(entity_names, i), name) == 0 ) {
      list_pop_at(entity_names, i);
    }
  }
  
  if (entity_exists(name)) {
    error("Don't know how to delete entity %s. No handler for type %s!", name, type_id_name(type_id));
  }
  
}

char* entity_name(entity* e) {
  
  int i;
  for(i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    entity* ent = dictionary_get(entities, name);
    
    if (ent == e) {
      return name;
    }
  }
  
  warning("Object at %p not loaded into entity manager. Cannot fetch name.", e);
  
  return NULL;
}

void entities_new_type_id(const char* name_format, int count, int type_id) {
  
  const int max_length = 1024;
  char entity_name[max_length];
  
  if(strlen(name_format) - 2 + ((count+1)/10) > max_length) {
    error("Name pattern and count are potentially longer than %i characters. Wont fit in buffer.", max_length);
  }
  
  if(!strstr(name_format, "%i")) {
    error("Name format must be like a sprintf format string and contain a %%i symbol for the entity index. E.G \"entity_%%i\"");
  }
  
  int i;
  for( i = 0; i < count; i++) {
    sprintf(entity_name, name_format, i);
    entity_new_type_id(entity_name, type_id);
  }
  
}

void entities_get_type_id(entity** out, int* returned, int type_id) {
  
  int count = 0;
  
  int i;
  for(i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type = dictionary_get(entity_types, name);
    entity* ent = dictionary_get(entities, name);
    
    if (*type == type_id) {
      out[count] = ent;
      count++;
    }
  }
  
  if (returned != NULL) {
    *returned = count;
  }
  
}
