#include "assets/material.h"

void material_entry_delete(material_entry* me) {
  SDL_GL_CheckError();
  shader_program_delete(me->program);
  for(int i = 0; i < me->num_items; i++) {
    free(me->names[i]);
  }
  free(me->names);
  free(me->types);
  free(me->items);
  free(me);
  SDL_GL_CheckError();
}

material_item material_entry_item(material_entry* me, char* name) {
  
  SDL_GL_CheckError();
  for(int i = 0; i < me->num_items; i++) {
    if (strcmp(me->names[i], name) == 0) {
      return me->items[i];
    }
  }
  
  material_item empty;
  memset(&empty, 0, sizeof(empty));
  
  SDL_GL_CheckError();
  return empty;
}

bool material_entry_has_item(material_entry* me, char* name) {
  SDL_GL_CheckError();
  for(int i = 0; i < me->num_items; i++) {
    if (strcmp(me->names[i], name) == 0) {
      return true;
    }
  }
  
  SDL_GL_CheckError();
  return false;
}

material* material_new() {
  material* m = malloc(sizeof(material));
  m->num_entries = 0;
  m->entries = NULL;
  return m;
}

void material_delete(material* m) {
  SDL_GL_CheckError();
  for(int i = 0; i < m->num_entries; i++) {
    material_entry_delete(m->entries[i]);
  }
  free(m->entries);
  free(m);
  SDL_GL_CheckError();
}

static void material_generate_programs(material* m) {

  SDL_GL_CheckError();
  
  for(int i = 0; i < m->num_entries; i++) {

    SDL_GL_CheckError();
  
    material_entry* me = m->entries[i];
    me->program = shader_program_new();
    
    SDL_GL_CheckError();
    bool attached = false;
    for(int j = 0; j < me->num_items; j++) {
      
      if (me->types[j] == mat_item_shader) {
        asset_hndl ah = me->items[j].as_asset;
        
        SDL_GL_CheckError();
        shader_program_attach_shader(me->program, asset_hndl_ptr(ah));
        attached = true;
        SDL_GL_CheckError();
      }
      
    }
    
    SDL_GL_CheckError();
    if (attached) { shader_program_link(me->program); }
    SDL_GL_CheckError();
    
  }
  
  SDL_GL_CheckError();
  
}

void material_entry_add_item(material_entry* me, char* name, int type, material_item mi) {
  SDL_GL_CheckError();
  me->num_items++;
  
  me->types = realloc(me->types, sizeof(int) * me->num_items);
  me->names = realloc(me->names, sizeof(char*) * me->num_items);
  me->items = realloc(me->items, sizeof(material_item) * me->num_items);
  
  me->items[me->num_items-1] = mi;
  me->types[me->num_items-1] = type;
  me->names[me->num_items-1] = malloc(strlen(name)+1);
  strcpy(me->names[me->num_items-1], name);  
  SDL_GL_CheckError();
}

material_entry* material_add_entry(material* m) {
  
  m->num_entries++;
  m->entries = realloc(m->entries, sizeof(material_entry*) * m->num_entries);
  m->entries[m->num_entries-1] = malloc(sizeof(material_entry));
  
  material_entry* me = m->entries[m->num_entries-1];
  me->program = NULL;
  me->num_items = 0;
  me->types = malloc(sizeof(int) * me->num_items);
  me->names = malloc(sizeof(char*) * me->num_items);
  me->items = malloc(sizeof(material_item) * me->num_items);
  
  return me;
}

material* mat_load_file(char* filename) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  material* m = material_new();
  material_entry* me = material_add_entry(m);
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (line[0] == '#') { continue; }
    if (line[0] == '\r') { continue; }
    if (line[0] == '\n') { continue; }
    
    if (strstr(line, "submaterial")) {
      
      /* Skip Empty Submaterials */
      if (me->num_items == 0) {
        continue;
      } else {
        me = material_add_entry(m);
        continue;
      }
      
    }
    
    char type[512]; char name[512]; char value[512];
    int matches = sscanf(line, "%511s %511s = %511s", type, name, value);
    
    if (matches != 3) continue;
    
    material_item mi;
    int type_id;
    char* end;
    float f0, f1, f2, f3;
    
    if (strcmp(type, "shader") == 0) {
    
      mi.as_asset = asset_hndl_new_load(P(value));
      type_id = mat_item_shader;
      
    } else if (strcmp(type, "texture") == 0) {
    
      mi.as_asset = asset_hndl_new_load(P(value));
      type_id = mat_item_texture;
    
    } else if (strcmp(type, "int") == 0) {
    
      mi.as_int = atoi(value);
      type_id = mat_item_int;
    
    } else if (strcmp(type, "float") == 0) {
      
      mi.as_float = atof(value);
      type_id = mat_item_float;
      
    } else if (strcmp(type, "vec2") == 0) {
    
      f0 = strtod(value, &end); f1 = strtod(end, NULL);
      mi.as_vec2 = vec2_new(f0, f1);
      type_id = mat_item_vec2;
      
    } else if (strcmp(type, "vec3") == 0) {
      
      f0 = strtod(value, &end); f1 = strtod(end, &end);
      f2 = strtod(end, NULL);
      mi.as_vec3 = vec3_new(f0, f1, f2);
      type_id = mat_item_vec3;
      
    } else if (strcmp(type, "vec4") == 0) {
    
      f0 = strtod(value, &end); f1 = strtod(end, &end);
      f2 = strtod(end, &end); f3 = strtod(end, NULL);
      mi.as_vec4 = vec4_new(f0, f1, f2, f3);
      type_id = mat_item_vec4;
      
    } else {
      error("Unknown material item type '%s'", type);
      return NULL;
    }
    
    material_entry_add_item(me, name, type_id, mi);
  
  }
  
  SDL_RWclose(file);
  
  material_generate_programs(m);
  
  return m;
}

material_entry* material_get_entry(material* m, int index) {
  return m->entries[(int)clamp(index, 0, m->num_entries-1)];
}

shader_program* material_first_program(material* m) {
  if (m->num_entries <= 0) {
    error("No entries in material!");
    return NULL;
  } else {
    return m->entries[0]->program;
  }
}
