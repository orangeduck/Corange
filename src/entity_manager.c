#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "list.h"
#include "camera.h"
#include "light.h"
#include "static_object.h"

#include "asset_manager.h"

#include "entity_manager.h"

list* entity_names;
dictionary* entities;
dictionary* entity_types;

void entity_manager_init() {
  
  entities = dictionary_new(512);
  entity_types = dictionary_new(512);
  
  entity_names = list_new(512);
}

void entity_manager_finish() {
    
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    printf("Deleting Entity %s\n", name);
    entity_delete(name);
  }
  
  list_delete_with(entity_names, free);
  
  dictionary_delete(entities);
  
  dictionary_map(entity_types, free);
  dictionary_delete(entity_types);
  
}

int entity_exists(char* name) {
  return dictionary_contains(entities, name);
}

entity* entity_new(char* name, int type) {

  if ( dictionary_contains(entities, name) ) {
    printf("Warning: Entity Manager already contains entity called %s! Not added.\n", name);
    return;
  }
  
  entity* e;
  
  if (type == entity_type_none) {
    e = NULL;
    
  } else if (type == entity_type_camera) {
    e = camera_new(v3_zero(), v3_one());
    
  } else if (type == entity_type_light) {
    e = light_new(v3_zero());
    
  } else if (type == entity_type_static) {
    e = static_object_new(NULL);
    
  } else {
    printf("Error: Don't know how to create entity %s. Unknown type id %i!\n", name, type);
    exit(EXIT_FAILURE);
  }
  
  dictionary_set(entities, name, e);
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type;
  
  dictionary_set(entity_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(entity_names, name_copy);

  return e;
}

void entity_add(char* name, int type, entity* entity) {

  if ( entity_exists(name) ) {
    printf("Warning: Entity Manager already contains entity called %s! Not added.\n", name);
    return;
  }
  
  dictionary_set(entities, name, entity);
  int* type_ptr = malloc(sizeof(int));
  *type_ptr = type;
  
  dictionary_set(entity_types, name, type_ptr);
  
  char* name_copy = malloc(strlen(name) + 1);
  strcpy(name_copy, name);
  list_push_back(entity_names, name_copy);
}

entity* entity_get(char* name) {
  
  if ( !entity_exists(name) ) {
    printf("Error: Entity %s does not exist!", name);
    exit(EXIT_FAILURE);
  }
  
  return dictionary_get(entities, name);
  
}

void entity_delete(char* name) {
  
  int* type_ptr = dictionary_get(entity_types, name);
  int type = *type_ptr;

  if (type == entity_type_none) {
    dictionary_remove_with(entities, name, free);
    
  } else if (type == entity_type_camera) {
    dictionary_remove_with(entities, name, (void (*)(void *))camera_delete);
    
  } else if (type == entity_type_light) {
    dictionary_remove_with(entities, name, (void (*)(void *))light_delete);
    
  } else if (type == entity_type_static) {
   dictionary_remove_with(entities, name, (void (*)(void *))static_object_delete);
    
  } else {
    printf("Error: Don't know how to delete entity %s. Unknown type id %i!\n", name, type);
    exit(EXIT_FAILURE);
  }
  
}

list* entities_get_all() {
  
  list* ret = list_new();
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    list_push_back(ret, dictionary_get(entities, name));
  }
  return ret;
  
}

list* entities_get_cameras() {

  list* ret = list_new();
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type_ptr = dictionary_get(entity_types, name);
    int type = *type_ptr;
    
    if (type == entity_type_camera) {
      list_push_back(ret, dictionary_get(entities, name));
    }
    
  }
  return ret;
  
}

list* entities_get_lights() {

  list* ret = list_new();
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type_ptr = dictionary_get(entity_types, name);
    int type = *type_ptr;
    
    if (type == entity_type_light) {
      list_push_back(ret, dictionary_get(entities, name));
    }
    
  }
  return ret;
  
}

list* entities_get_statics() {

  list* ret = list_new();
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type_ptr = dictionary_get(entity_types, name);
    int type = *type_ptr;
    
    if (type == entity_type_static) {
      list_push_back(ret, dictionary_get(entities, name));
    }
    
  }
  return ret;

}

list* entities_get_type(int type) {

  list* ret = list_new();
  int i;
  for (i = 0; i < entity_names->num_items; i++) {
    char* name = list_get(entity_names, i);
    int* type_ptr = dictionary_get(entity_types, name);
    int i_type = *type_ptr;
    
    if (i_type == type) {
      list_push_back(ret, dictionary_get(entities, name));
    }
    
  }
  return ret;
  
}