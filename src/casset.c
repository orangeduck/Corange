#include "casset.h"

#include "data/dict.h"
#include "data/list.h"

static dict* asset_dict;

typedef struct {
  type_id type;
  char* extension;
  void* (*load_func)(const char*);
  void (*del_func)();
} asset_handler;

#define MAX_ASSET_HANDLERS 512
static asset_handler asset_handlers[MAX_ASSET_HANDLERS];
static int num_asset_handlers = 0;

typedef struct {
  fpath variable;
  fpath mapping;
} path_variable;

#define MAX_PATH_VARIABLES 512
static path_variable path_variables[MAX_PATH_VARIABLES];
static int num_path_variables = 0;

void asset_add_path_variable(fpath variable, fpath mapping) {
  
  if (num_path_variables == MAX_PATH_VARIABLES) {
    error("Already reached maximum num of path variables (%i)", MAX_PATH_VARIABLES);
  }
  
  if (variable.ptr[0] != '$') {
    error("Variables must start with a dollar sign e.g '$CORANGE'");
  }
  
  path_variable pv = { variable, mapping }; 
  
  path_variables[num_path_variables] = pv;
  num_path_variables++;
  
}

static fpath asset_map_realpath(fpath filename) {
  fpath out;
  SDL_PathFullName(out.ptr, filename.ptr);
  return out;
}

static fpath asset_map_filename(fpath filename) {
  
  fpath out = filename;
  
  for(int i = 0; i < num_path_variables; i++) {
  
    fpath variable = path_variables[i].variable;
    fpath mapping = path_variables[i].mapping;
    
    char* subptr = strstr(out.ptr, variable.ptr);
    
    if (subptr) {
      
      fpath sub; strcpy(sub.ptr, subptr);
    
      int replace_len = strlen(mapping.ptr);
      int start_len = strlen(out.ptr) - strlen(sub.ptr);
      int ext_len = strlen(sub.ptr) - strlen(variable.ptr);
      
      out.ptr[start_len] = '\0';
      strcat(out.ptr, mapping.ptr);
      strcat(out.ptr, sub.ptr + strlen(variable.ptr));
    }
  
  }

  return asset_map_realpath(out);
}

asset_hndl asset_hndl_null() {
  asset_hndl ah;
  ah.path = P("");
  ah.ptr = NULL;
  return ah;
}

asset_hndl asset_hndl_new(fpath path) {
  asset_hndl ah;
  ah.path = asset_map_filename(path);
  ah.ptr = NULL;
  return ah;
}

asset_hndl asset_hndl_new_load(fpath path) {
  asset_hndl ah = asset_hndl_new(path);
  if (!file_isloaded(ah.path)) {
    file_load(ah.path);
  }
  return ah;
}

asset_hndl asset_hndl_new_ptr(asset* as) {
  asset_hndl ah;
  ah.path = P(asset_ptr_path(as));
  ah.ptr = as;
  return ah;
}

bool asset_hndl_isnull(asset_hndl ah) {
  return (strcmp(ah.path.ptr, "") == 0);
}

fpath asset_hndl_path(asset_hndl ah) {
  return ah.path;
}

asset* asset_hndl_ptr(asset_hndl ah) {

  if (strcmp(ah.path.ptr, "") == 0) {
    error("Cannot load NULL asset handle");
  }

  ah.ptr = dict_get(asset_dict, ah.path.ptr);
  
  if (ah.ptr == NULL) {
    error("Failed to get Asset '%s', is it loaded yet?", ah.path.ptr);
  }
  
  return ah.ptr;
}

void asset_init(char* game_name) {
  asset_dict = dict_new(1024);
}

void asset_handler_delete(asset_handler* h) {

  free(h->extension);
  free(h);

}

static void delete_bucket_list(struct bucket* b) {
  
  if(b == NULL) {
    return;
  }
  
  delete_bucket_list(b->next);
  
  debug("Unloading: '%s'", b->string);
  
  fpath ext;
  SDL_PathFileExtension(ext.ptr, b->string);
  
  for(int i = 0; i < num_asset_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext.ptr, handler.extension) == 0) {
      
      bucket_delete_with(b, handler.del_func);
      
      break;
    }
    
  }
  
}

void asset_finish() {

  for(int i=0; i <asset_dict->size; i++) {
    struct bucket* b = asset_dict->buckets[i];
    delete_bucket_list(b);
  }
  
  for(int i=0; i < num_asset_handlers; i++) {
    free(asset_handlers[num_asset_handlers].extension);
  }
  
}

void asset_handler_cast(type_id type, const char* extension, void* asset_loader(const char* filename) , void asset_deleter(void* asset) ) {
  
  if(num_asset_handlers == MAX_ASSET_HANDLERS) {
    warning("Max number of asset handlers reached. Handler for extension '%s' not added.", extension);
    return;
  }
  
  asset_handler h;
  char* c = malloc(strlen(extension) + 1);
  strcpy(c, extension);
  h.type = type;
  h.extension = c;
  h.load_func = asset_loader;
  h.del_func = asset_deleter;

  asset_handlers[num_asset_handlers] = h;
  num_asset_handlers++;
  
}

void file_load(fpath filename) {
  
  filename = asset_map_filename(filename);
  
  if (dict_contains(asset_dict, filename.ptr)) {
    error("Asset '%s' already loaded", filename.ptr);
  }
  
  fpath ext;
  SDL_PathFileExtension(ext.ptr, filename.ptr);
  
  for(int i=0; i < num_asset_handlers; i++) {
    asset_handler handler = asset_handlers[i];
    
    if (strcmp(ext.ptr, handler.extension) == 0) {
      debug("Loading: '%s'", filename.ptr);
      asset* a = handler.load_func(filename.ptr);
      dict_set(asset_dict, filename.ptr, a);
      break;
    }
    
  }

}

void folder_load(fpath folder) {
  
  folder = asset_map_filename(folder);
  debug("Loading Folder: '%s'", folder.ptr);
  
  DIR* dir = opendir(folder.ptr);
  if (dir == NULL) {
    error("Could not open directory '%s' to load.", folder.ptr);
  }
  
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
  
    if ((strcmp(ent->d_name,".") != 0) && 
        (strcmp(ent->d_name,"..") != 0)) {
    
      fpath filename = folder;
      
      // If does not end in "/" then copy it.
      if (folder.ptr[strlen(folder.ptr)-1] != '/') {
        strcat(filename.ptr, "/");
      }
      
      strcat(filename.ptr, ent->d_name);
      
      if (!file_isloaded(filename)) {
        file_load(filename);
      }
    } 
  }
  
  closedir(dir);
}

void file_reload(fpath filename) {
  file_unload(filename);
  file_load(filename);
}

void folder_reload(fpath folder) {
  folder_unload(folder);
  folder_load(folder);
}

void file_unload(fpath filename) {
  
  filename = asset_map_filename(filename);
  
  fpath ext;
  SDL_PathFileExtension(ext.ptr, filename.ptr);
  
  for(int i=0; i < num_asset_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext.ptr, handler.extension) == 0) {
      debug("Unloading: '%s'", filename.ptr);
      dict_remove_with(asset_dict, filename.ptr, handler.del_func);
      break;
    }
    
  }
}

void folder_unload(fpath folder) {
    
  folder = asset_map_filename(folder);
  
  debug("Unloading Folder: '%s'", folder.ptr);
  DIR* dir = opendir(folder.ptr);
  
  if (dir == NULL) {
    error("Could not open directory '%s' to unload.\n", folder.ptr);
  }
  
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
  
    if ((strcmp(ent->d_name,".") != 0) && 
        (strcmp(ent->d_name,"..") != 0)) {
    
      fpath filename = folder;
      strcat(filename.ptr, ent->d_name);
      
      if(dict_contains(asset_dict, filename.ptr) ) {
        file_unload(filename);
      }
      
    } 
  }
  closedir(dir);
}

bool file_isloaded(fpath path) {
  path = asset_map_filename(path);
  return dict_contains(asset_dict, path.ptr);
}

void asset_reload_type_id(type_id type) {

}

void asset_reload_all() {
  
  debug("Reloading All Assets...");
  
  list* asset_names = list_new();
  
  for(int i = 0; i < asset_dict->size; i++) {
    struct bucket* b = asset_dict->buckets[i];
    while(b != NULL) {
      char* new_name = malloc(strlen(b->string)+1);
      strcpy(new_name, b->string);
      list_push_back(asset_names, new_name);
      b = b->next;
    }
  }
  
  for(int i = 0; i < asset_names->num_items; i++) {
    file_unload(P(list_get(asset_names, i)));
  }
  
  for(int i = 0; i < asset_names->num_items; i++) {
    /*
    ** Assets can reload their child assets before we do
    ** So it is important we check before loading again
    */
    if (!file_isloaded(P(list_get(asset_names, i)))) {
      file_load(P(list_get(asset_names, i)));
    }
  }
  
  list_delete_with(asset_names, free);
}

char* asset_ptr_path(asset* a) {
  char* path = dict_find(asset_dict, a);
  if (path == NULL) {
    error("Asset dict doesn't contain asset pointer %p", a);
    return NULL;
  } else {
    return path; 
  }
}

char* asset_ptr_typename(asset* a) {
  char* path = dict_find(asset_dict, a);
  if (path == NULL) {
    error("Asset dict doesn't contain asset pointer %p", a);
    return NULL;
  }
  
  fpath ext;
  SDL_PathFileExtension(ext.ptr, path);
  
  for(int i=0; i < num_asset_handlers; i++) {
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext.ptr, handler.extension) == 0) {
      return type_id_name(handler.type);
    }
  }
  
  return NULL;
}