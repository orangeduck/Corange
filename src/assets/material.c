#include "assets/material.h"

void material_entry_delete(material_entry* me) {
  shader_program_delete(me->program);
  for(int i = 0; i < me->num_items; i++) {
    free(me->names[i]);
  }
  free(me->names);
  free(me->types);
  free(me->items);
  free(me);
}

material_item material_entry_item(material_entry* me, char* name) {
  
  for(int i = 0; i < me->num_items; i++) {
    if (strcmp(me->names[i], name) == 0) {
      return me->items[i];
    }
  }
  
  material_item empty;
  memset(&empty, 0, sizeof(empty));
  
  return empty;
}

bool material_entry_has_item(material_entry* me, char* name) {
  for(int i = 0; i < me->num_items; i++) {
    if (strcmp(me->names[i], name) == 0) {
      return true;
    }
  }
  
  return false;
}

material* material_new() {
  material* m = malloc(sizeof(material));
  m->num_entries = 0;
  m->entries = malloc(sizeof(material_entry) * m->num_entries);
  return m;
}

void material_delete(material* m) {
  for(int i = 0; i < m->num_entries; i++) {
    material_entry_delete(m->entries[i]);
  }
  free(m->entries);
  free(m);
}

material* mat_load_file(char* filename) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  material* m = malloc(sizeof(material));
  m->num_entries = 1;
  m->entries = malloc(sizeof(material_entry) * m->num_entries);
  
  material_entry* me = m->entries[m->num_entries];
  me->num_items = 0;
  me->types = realloc(me->types, sizeof(int) * me->num_items);
  me->names = realloc(me->names, sizeof(char*) * me->num_items);
  me->items = realloc(me->items, sizeof(material_item) * me->num_items);
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    material_entry* me = m->entries[m->num_entries];
    
    me->num_items++;
    me->types = realloc(me->types, sizeof(int) * me->num_items);
    me->names = realloc(me->names, sizeof(char*) * me->num_items);
    me->items = realloc(me->items, sizeof(material_item) * me->num_items);
    
    char type[256]; char name[256]; char value[256];
    sscanf(line, "%255s %255s=%255s", type, name, value);
    
    me->names[me->num_items] = malloc(strlen(name)+1);
    strcpy(me->names[me->num_items], name);
    
    material_item mi;
    
    if (strcmp(type, "shader") == 0) {
      mi.as_asset = asset_hndl_new(P(value));
      me->types[me->num_items] = mat_item_shader;
    } else if (strcmp(type, "texture") == 0) {
      mi.as_asset = asset_hndl_new(P(value));
      me->types[me->num_items] = mat_item_texture;
    } else if (strcmp(type, "float") == 0) {
      mi.as_float = 1.0f;
      me->types[me->num_items] = mat_item_float;
    } else if (strcmp(type, "vec2") == 0) {
      mi.as_vec2 = vec2_new(1.0f, 1.0f);
      me->types[me->num_items] = mat_item_vec2;
    } else if (strcmp(type, "vec3") == 0) {
      mi.as_vec3 = vec3_new(1.0f, 1.0f, 1.0f);
      me->types[me->num_items] = mat_item_vec3;
    } else if (strcmp(type, "vec4") == 0) {
      mi.as_vec4 = vec4_new(1.0f, 1.0f, 1.0f, 1.0f);
      me->types[me->num_items] = mat_item_vec4;
    } else {
      error("Unknown material item type '%s'", type);
    }
    
    me->items[me->num_items] = mi;
  
  }
  
  SDL_RWclose(file);
  
  return m;
}

material_entry* material_get_entry(material* m, int index) {
  return m->entries[index];
}
