#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include "SDL/SDL_rwops.h"

#include "asset_manager.h"

static dictionary* asset_dictionary;

typedef struct {

  char* extension;
  void* (*load_func)();
  void (*del_func)();

} asset_handler;

static asset_handler asset_handlers[512];
static int num_handlers = 0;

static char* asset_manager_game_name;

char* asset_map_filename(char* filename) {
  
  /* Dot in front means relative path - use as is */
  
  if (filename[0] == '.') { return filename; }
  
  /* Slash means absolute path, assume to be in game directory */
  
  if (filename[0] == '/') {
    
    char* new_filename = malloc( strlen("./games/") + strlen(filename) +
                                 strlen(asset_manager_game_name) + 1);
    
    strcpy(new_filename, "./games/");
    strcat(new_filename, asset_manager_game_name);
    strcat(new_filename, filename);
    
    free(filename);
    filename = new_filename;
        
    //printf("Mapped to: %s\n", new_filename);
        
    return new_filename;
    
  } else {
    printf("Warning: Unsure how to convert path '%s' into a real path.\n", filename);
    return filename;
  }

}

void asset_manager_init(char* game_name) {

  asset_manager_game_name = game_name;
  printf("Creating new asset manager\n");
  asset_dictionary = dictionary_new(1024);

}

void asset_handler_delete(asset_handler* h) {

  free(h->extension);
  free(h);

}

/* Unloads all assets, clears the stringtable with the supplied handlers */
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

void delete_bucket_list(bucket* b) {
  
  if(b == NULL) {
    return;
  }
  
  delete_bucket_list(b->next);
  
  printf("Deleting %s...\n", b->string); fflush(stdout);
  
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


void asset_manager_handler(char* extension, void* load_func(char*) , void del_func(void*) ) {
  
  asset_handler h;
  char* c = malloc(strlen(extension) + 1);
  strcpy(c, extension);
  h.extension = c;
  h.load_func = load_func;
  h.del_func = del_func;

  asset_handlers[num_handlers] = h;
  num_handlers++;
  
};

void load_file(char* filename) {
  
  filename = asset_map_filename(filename);
  
  if (dictionary_contains(asset_dictionary, filename)) {
    printf("Asset %s already loaded\n", filename);
    return;
  }
  char* ext = asset_file_extension(filename);
  int i;
  for(i=0; i < num_handlers; i++) {
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      void* asset = handler.load_func(filename);
      dictionary_set(asset_dictionary, filename, asset);
      break;
    }
  }
  free(ext);
  
  return;

};

void load_folder(char* folder) {
    
  folder = asset_map_filename(folder);
    
  printf("\n\t---- Loading Folder %s ----\n\n", folder); fflush(stdout);
  
  DIR* dir = opendir(folder);
  struct dirent* ent;
  
  if (dir != NULL) {
    
    while ((ent = readdir(dir)) != NULL) {
    
      if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
      
        char* filename = malloc(strlen(folder) + strlen(ent->d_name) + 1);
        strcpy(filename, folder);
        strcat(filename, ent->d_name);
        
        load_file(filename);
        
        free(filename);
      } 
    }
    closedir(dir);
    printf("\n\n"); fflush(stdout);
  
  } else {
    printf("Error: Could not open directory %s\n", folder); fflush(stdout);
  }
  
};

void reload_file(char* filename) {

  filename = asset_map_filename(filename);

  if (dictionary_contains(asset_dictionary, filename)) {
    unload_file(filename);
  }
  
  load_file(filename);
  
};

void reload_folder(char* folder) {

  folder = asset_map_filename(folder);

  unload_folder(folder);
  load_folder(folder);
};

void unload_file(char* filename) {
  
  filename = asset_map_filename(filename);
  
  char* ext = asset_file_extension(filename);
  int i;
  for(i=0; i < num_handlers; i++) {
  
    asset_handler handler = asset_handlers[i];
    if (strcmp(ext, handler.extension) == 0) {
      dictionary_remove_with(asset_dictionary, filename, handler.del_func);
      break;
    }
    
  }
  
  free(ext);
  
};

void unload_folder(char* folder) {
    
  folder = asset_map_filename(folder);
  
  DIR* dir = opendir(folder);
  struct dirent* ent;
  
  if (dir != NULL) {
    
    while ((ent = readdir(dir)) != NULL) {
    
      if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
      
        char* filename = malloc(strlen(folder) + strlen(ent->d_name) + 1);
        strcpy(filename, folder);
        strcat(filename, ent->d_name);
        
        if(dictionary_contains(asset_dictionary, filename) ) {
          unload_file(filename);
        }
        
        free(filename);
      } 
    }
    closedir(dir);
    printf("\n\n"); fflush(stdout);
  
  } else {
    printf("Error: Could not open directory %s\n", folder); fflush(stdout);
  }

};

void* asset_get(char* path) {
  path = asset_map_filename(path);
  void* val = dictionary_get(asset_dictionary, path);
  if (val == NULL) {
    printf("Error: Could not find asset %s. Perhaps it is not loaded yet?", path);
    exit(EXIT_FAILURE);
  };
  return val;
};

int asset_loaded(char* path) {
  path = asset_map_filename(path);
  return dictionary_contains(asset_dictionary, path);
}

/* Asset Loader helper commands */

char* asset_load_file(char* filename) {
  
  filename = asset_map_filename(filename);
  
  printf("Loading: %s\n", filename); fflush(stdout);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    printf("Error Loading File %s: Can't find file.\n", filename); fflush(stdout);
    return NULL;
  }
  
  long size = SDL_RWseek(file,0,SEEK_END);
  char* contents = malloc(size+1);
  contents[size] = '\0';
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, size, 1);
  
  SDL_RWclose(file);
  
  return contents;
  
};

char* asset_file_extension(char* filename) {
  
  filename = asset_map_filename(filename);
  
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
};

char* asset_file_location(char* filename) {

  filename = asset_map_filename(filename);

  int len = strlen(filename);
  int i = len;
  while( i > 0) {
    
    if (filename[i] != '/') { len--; }
    if (filename[i] == '/') { break; }
  
    i--;
  }
  i++;
  len++;
  
  char* main = malloc(len+1);
  memcpy(main, filename, len);
  main[len] = '\0';
  
  return main;
  
}