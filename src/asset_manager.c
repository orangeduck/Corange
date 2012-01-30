#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include "corange.h"

static dictionary* asset_dictionary;

typedef struct {

  char* extension;
  void* (*load_func)();
  void (*del_func)();

} asset_handler;

#define MAX_ASSET_HANDLERS 512
static asset_handler asset_handlers[MAX_ASSET_HANDLERS];
static int num_asset_handlers = 0;

typedef struct {
  char* variable;
  char* mapping;
} path_variable;

#define MAX_PATH_VARIABLES 512
static path_variable path_variables[MAX_PATH_VARIABLES];
static int num_path_variables = 0;

/*
  This whole string situation here is a bit of a mess.
  And I'm not sure how possible it is to do without memory allocation.
  The issue is we have strings coming in from the stack and the heap.
  We can't statically allocate memory for asset_map_filename because
  there are too many situations where it is called to multiple stack depth.
  
  There is almost certainly a memory leak somewhere in this code.
  And the whole thing could do with some sanity. But I think it works for now.
  
  The solution is probably to make asset_map_filename static and flatten
  its use across all the functions.
*/

void asset_manager_add_path_variable(char* variable, char* mapping) {
  
  if (num_path_variables == MAX_PATH_VARIABLES) {
    error("Already reached maximum num of path variables (%i)", MAX_PATH_VARIABLES);
  }
  
  if (variable[0] != '$') {
    error("Variables must start with a dollar sign e.g '$CORANGE'");
  }
  
  path_variable pv;
  pv.variable = malloc(strlen(variable) + 1);
  strcpy(pv.variable, variable);
  pv.mapping = malloc(strlen(mapping) + 1);
  strcpy(pv.mapping, mapping);
  
  path_variables[num_path_variables] = pv;
  num_path_variables++;
  
}

static char* asset_map_realpath(char* filename) {

  char* actualpath = malloc(MAX_PATH);
  
  SDL_PathFullName(actualpath, filename);
  free(filename);
  
  return actualpath;
}

char* asset_map_filename(char* filename) {
  
  for(int i = 0; i < num_path_variables; i++) {
  
    char* variable = path_variables[i].variable;
    char* mapping = path_variables[i].mapping;
  
    char* sub = strstr(filename, variable);
    
    if (sub) {
      int replace_len = strlen(mapping);
      int start_len = strlen(filename) - strlen(sub);
      int ext_len = strlen(sub) - strlen(variable);
      
      char* new_filename = malloc(replace_len + ext_len + start_len + 1);
      new_filename[0] = '\0';
      
      strncpy(new_filename, filename, start_len);
      strcat(new_filename, mapping);
      strcat(new_filename, sub + strlen(variable));
      
      return asset_map_realpath(new_filename);
    }
  
  }
  
  char* new_filename = malloc(strlen(filename) + 1);
  strcpy(new_filename, filename);

  return asset_map_realpath(new_filename);
  
}

void asset_manager_init(char* game_name) {

  asset_dictionary = dictionary_new(1024);

}

void asset_handler_delete(asset_handler* h) {

  free(h->extension);
  free(h);

}

static void delete_bucket_list(bucket* b) {
  
  if(b == NULL) {
    return;
  }
  
  delete_bucket_list(b->next);
  
  debug("Unloading: %s", b->string);
  
  char* ext = asset_name_extension(b->string);
  
  for(int i = 0; i < num_asset_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      
      bucket_delete_with(b, handler.del_func);
      
      break;
    }
    
  }
  
  free(ext);
  
}

void asset_manager_finish() {

  for(int i=0; i <asset_dictionary->table_size; i++) {
    bucket* b = asset_dictionary->buckets[i];
    delete_bucket_list(b);
  }
  
  for(int i=0; i < num_asset_handlers; i++) {
    free(asset_handlers[num_asset_handlers].extension);
  }
  
  for(int i = 0; i < num_path_variables; i++) {
    free(path_variables[i].variable);
    free(path_variables[i].mapping);
  }
  
}

void asset_manager_handler_cast(char* extension, void* asset_loader(char* filename) , void asset_deleter(void* asset) ) {
  
  if(num_asset_handlers == MAX_ASSET_HANDLERS) {
    warning("Max number of asset handlers reached. Handler for extension %s not added.", extension);
    return;
  }
  
  asset_handler h;
  char* c = malloc(strlen(extension) + 1);
  strcpy(c, extension);
  h.extension = c;
  h.load_func = asset_loader;
  h.del_func = asset_deleter;

  asset_handlers[num_asset_handlers] = h;
  num_asset_handlers++;
  
}

void load_file(char* filename) {
  
  char* filename_map = asset_map_filename(filename);
  
  if (dictionary_contains(asset_dictionary, filename_map)) {
    error("Asset %s already loaded", filename_map);
  }
  
  char* ext = asset_name_extension(filename_map);
  
  for(int i=0; i < num_asset_handlers; i++) {
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      debug("Loading: %s", filename_map);
      void* asset = handler.load_func(filename_map);
      dictionary_set(asset_dictionary, filename_map, asset);
      break;
    }
  }
  
  free(ext);
  free(filename_map);

}

void load_folder(char* folder) {
    
  char* folder_map = asset_map_filename(folder);
  
  debug("Loading Folder: %s", folder_map);
  
  DIR* dir = opendir(folder_map);
  struct dirent* ent;
  
  if (dir == NULL) {
    error("Could not open directory %s.", folder_map);
  }
    
  while ((ent = readdir(dir)) != NULL) {
  
    if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
    
      char* filename = malloc(MAX_PATH);
      strcpy(filename, folder_map);
      if (folder_map[strlen(folder_map)-1] != '/') {
        strcat(filename, "/");
      }
      strcat(filename, ent->d_name);
      
      if (!asset_loaded(filename)) {
        load_file(filename);
      }
      
      free(filename);
    } 
  }
  closedir(dir);
  
  free(folder_map);
  
}

void reload_file(char* filename) {
  unload_file(filename);
  load_file(filename);
}

void reload_folder(char* folder) {
  unload_folder(folder);
  load_folder(folder);
}

void unload_file(char* filename) {
  
  char* filename_map = asset_map_filename(filename);
  
  char* ext = asset_name_extension(filename_map);
  
  for(int i=0; i < num_asset_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      debug("Unloading: %s", filename_map);
      dictionary_remove_with(asset_dictionary, filename_map, handler.del_func);
      break;
    }
    
  }
  
  free(ext);
  free(filename_map);
}

void unload_folder(char* folder) {
    
  char* folder_map = asset_map_filename(folder);
  
  DIR* dir = opendir(folder_map);
  struct dirent* ent;
  
  if (dir == NULL) {
    error("Could not open directory %s\n", folder_map);
  }
  
  while ((ent = readdir(dir)) != NULL) {
  
    if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
    
      char* filename = malloc(strlen(folder_map) + strlen(ent->d_name) + 1);
      strcpy(filename, folder_map);
      strcat(filename, ent->d_name);
      
      if(dictionary_contains(asset_dictionary, filename) ) {
        unload_file(filename);
      }
      
      free(filename);
    } 
  }
  closedir(dir);
  
  free(folder_map);
}

void* asset_get(char* path) {
  char* path_map = asset_map_filename(path);
  void* val = dictionary_get(asset_dictionary, path_map);
  
  if (val == NULL) {
    error("Could not find asset %s. Perhaps it is not loaded yet?", path_map);
  }
  
  free(path_map);
  return val;
}

void* asset_load_get(char* path) {
  char* path_map = asset_map_filename(path);
  void* val = dictionary_get(asset_dictionary, path_map);
  if (val == NULL) {
    load_file(path_map);
    val = dictionary_get(asset_dictionary, path_map);
  }
  free(path_map);
  return val;
}

bool asset_loaded(char* path) {
  char* path_map = asset_map_filename(path);
  bool contains = dictionary_contains(asset_dictionary, path_map);
  free(path_map);
  return contains;
}

void asset_state_print() {
  dictionary_print(asset_dictionary);
}

/* Asset Loader helper commands */

char* asset_name_extension(char* filename) {
  
  int ext_len = 0;
  int i = strlen(filename);
  while( i >= 0) {
    
    if (filename[i] != '.') { ext_len++; }
    if (filename[i] == '.') { break; }
  
    i--;
  }
  
  char* ext = malloc(ext_len);
  
  int prev = strlen(filename) - ext_len + 1;
  char* f_ext = filename + prev;
  strcpy(ext, f_ext);
  
  return ext;
}

char* asset_name_location(char* filename) {
  
  int i = strlen(filename);
  while( i > 0) {
    
    if (filename[i] != '/') { i--; }
    if (filename[i] == '/') { break; }
    if (filename[i] == '\\') { break; }
  
    i--;
  }
  i++;
  
  char* loc = malloc(i+1);
  memcpy(loc, filename, i);
  loc[i] = '\0';
  
  return loc;
  
}
