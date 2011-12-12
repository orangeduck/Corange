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

#define MAX_HANDLERS 512
asset_handler asset_handlers[MAX_HANDLERS];
int num_handlers = 0;

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

char* asset_map_filename(char* filename) {
  
  char* sub = strstr(filename, "$CORANGE");
  char* corange_path = corange_asset_path();
  
  if (!sub || (corange_path == NULL)) {
  
    char* new_filename = malloc(strlen(filename) + 1);
    strcpy(new_filename, filename);
  
    return new_filename;
    
  } else {
    
    int replace_len = strlen(corange_asset_path());
    int start_len = strlen(filename) - strlen(sub);
    int ext_len = strlen(sub) - strlen("$CORANGE");
    
    char* new_filename = malloc(replace_len + ext_len + start_len + 1);
    new_filename[0] = '\0';
    
    strncpy(new_filename, filename, start_len);
    strcat(new_filename, corange_asset_path());
    strcat(new_filename, sub + strlen("$CORANGE"));
    
    return new_filename;
  }
  
}

void asset_manager_init(char* game_name) {

  printf("Creating new asset manager\n");
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
  
  printf("Unloading %s...\n", b->string);
  
  char* ext = asset_file_extension(b->string);
  
  int i;
  for(i=0; i < num_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      
      bucket_delete_with(b, handler.del_func);
      
      break;
    }
    
  }
  
  free(ext);
  
}

void asset_manager_finish() {

  int i;
  for(i=0; i <asset_dictionary->table_size; i++) {
    bucket* b = asset_dictionary->buckets[i];
    delete_bucket_list(b);
  }
  
  for(i=0; i < num_handlers; i++) {
    free(asset_handlers[num_handlers].extension);
  }
  
}

void asset_manager_handler(char* extension, void* asset_loader(char* filename) , void asset_deleter(void* asset) ) {
  
  if(num_handlers == MAX_HANDLERS) {
    warning("Max number of asset handlers reached. Handler for extension %s not added.", extension);
  }
  
  asset_handler h;
  char* c = malloc(strlen(extension) + 1);
  strcpy(c, extension);
  h.extension = c;
  h.load_func = asset_loader;
  h.del_func = asset_deleter;

  asset_handlers[num_handlers] = h;
  num_handlers++;
  
}

void load_file(char* filename) {
  
  char* filename_map = asset_map_filename(filename);
  
  if (dictionary_contains(asset_dictionary, filename_map)) {
    error("Asset %s already loaded", filename_map);
  }
  
  char* ext = asset_file_extension(filename_map);
  int i;
  for(i=0; i < num_handlers; i++) {
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      printf("Loading: %s\n", filename_map);
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
    
  printf("\n\t---- Loading Folder %s ----\n\n", folder_map);
  
  DIR* dir = opendir(folder_map);
  struct dirent* ent;
  
  if (dir == NULL) {
    error("Could not open directory %s.", folder_map);
  }
    
  while ((ent = readdir(dir)) != NULL) {
  
    if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
    
      char* filename = malloc(strlen(folder_map) + strlen(ent->d_name) + 1);
      strcpy(filename, folder_map);
      strcat(filename, ent->d_name);
      
      if (!asset_loaded(filename)) {
        load_file(filename);
      }
      
      free(filename);
    } 
  }
  closedir(dir);
  printf("\n\n"); fflush(stdout);
  
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
  
  char* ext = asset_file_extension(filename_map);
  int i;
  for(i=0; i < num_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      printf("Unloading: %s\n", filename_map);
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

int asset_loaded(char* path) {
  char* path_map = asset_map_filename(path);
  int contains = dictionary_contains(asset_dictionary, path_map);
  free(path_map);
  return contains;
}

void asset_state_print() {
  dictionary_print(asset_dictionary);
}

/* Asset Loader helper commands */

char* asset_file_contents(char* filename) {
  
  char* filename_map = asset_map_filename(filename);
  
  SDL_RWops* file = SDL_RWFromFile(filename_map, "r");
  
  if(file == NULL) {
    error("Cannot load file %s", filename_map);
  }
  
  long size = SDL_RWseek(file,0,SEEK_END);
  char* contents = malloc(size+1);
  contents[size] = '\0';
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, size, 1);
  
  SDL_RWclose(file);
  free(filename_map);
  
  return contents;
  
}

char* asset_file_extension(char* filename) {
  
  char* filename_map = asset_map_filename(filename);
  
  int ext_len = 0;
  int i = strlen(filename_map);
  while( i >= 0) {
    
    if (filename_map[i] != '.') { ext_len++; }
    if (filename_map[i] == '.') { break; }
  
    i--;
  }
  
  char* ext = malloc(ext_len);
  
  int prev = strlen(filename_map) - ext_len + 1;
  char* f_ext = filename_map + prev;
  strcpy(ext, f_ext);
  
  free(filename_map);
  
  return ext;
}

char* asset_file_location(char* filename) {

  char* filename_map = asset_map_filename(filename);
  
  int len = strlen(filename_map);
  int i = len;
  while( i > 0) {
    
    if (filename_map[i] != '/') { len--; }
    if (filename_map[i] == '/') { break; }
  
    i--;
  }
  i++;
  len++;
  
  char* loc = malloc(len+1);
  memcpy(loc, filename_map, len);
  loc[len] = '\0';
  
  free(filename_map);
  
  return loc;
  
}