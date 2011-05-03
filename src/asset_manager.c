#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include "SDL_rwops.h"

#include "stringtable.h"

#include "asset_manager.h"

static stringtable* asset_dictionary;

typedef struct {

  char* extension;
  void* (*load_func)();
  void (*del_func)();

} asset_handler;

static asset_handler asset_handlers[512];
static int num_handlers = 0;

void asset_manager_init() {

  printf("Creating new asset manager\n");
  asset_dictionary = stringtable_new(1024);

}

void asset_handler_delete(asset_handler* h) {

  free(h->extension);
  free(h);

}

void asset_manager_finish() {

  stringtable_delete(asset_dictionary);
  
  int i;
  for(i=0; i < num_handlers; i++) {
    free(asset_handlers[num_handlers].extension);
  }
  
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

void load_asset(char* filename) {
  
  char* ext = asset_file_extension(filename);
  
  int i;
  for(i=0; i < num_handlers; i++) {
    
    asset_handler hand = asset_handlers[i];
    if (strcmp(ext, hand.extension) == 0) {
      
      printf("Checking extension: %s\n", hand.extension); fflush(stdout);
      
      void* asset = hand.load_func(filename);
      
      stringtable_set(asset_dictionary, filename, asset);
      //printf("HELLO EVERYONE!\n");
      
    }
    
  }
  
  free(ext);
  
  return;

};

void load_folder(char* folder) {
    
    /*
    
    int str_size = strlen(folder);
    if (folder[str_size-2] != '\\') {
      folder = realloc(folder, str_size + 1);
      folder[str_size-1] = '\\';
      folder[str_size] = '\0';
    };
    
    */
    
    printf(" \t---- Loading Folder %s ----\n\n", folder); fflush(stdout);
    
    DIR* dir = opendir(folder);
    struct dirent* ent;
    
    if (dir != NULL) {
      
      /* print all the files and directories within directory */
      while ((ent = readdir(dir)) != NULL) {
      
        if ((strcmp(ent->d_name,".") != 0) && (strcmp(ent->d_name,"..") != 0)) {
        
          char* filename = malloc(strlen(folder) + strlen(ent->d_name) + 1);
          strcpy(filename, folder);
          strcat(filename, ent->d_name);
          
          printf("File: %s \n",ent->d_name); fflush(stdout);
          
          load_asset(filename);
          
          free(filename);
        } 
      }
      closedir(dir);
      printf(" \n\t---- Done ----\n\n", folder); fflush(stdout);
    
    } else {
      printf("Asset Load Error: Could not open directory %s\n", folder); fflush(stdout);
    }
  
};

void reload_asset(char* filename) {
  load_asset(filename);
};

void reload_folder(char* folder) {
  load_folder(folder);
};

void* asset_get(char* path) {
  return stringtable_get(asset_dictionary, path);
};


char* asset_load_file(char* filename) {
  
  printf("Loading File: %s\n", filename); fflush(stdout);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    printf("Error Loading File %s: Can't find file.\n", filename); fflush(stdout);
    return NULL;
  }
  
  long size = SDL_RWseek(file,0,SEEK_END);
  char* contents = malloc(size+1);
  contents[size] = '\0';
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, 1, size);
  
  SDL_RWclose(file);
  
  return contents;
  
};

char* asset_file_extension(char* filename) {
  
  int ext_len = 0;
  int i = strlen(filename);
  while( i > 0) {
    
    if (filename[i] != '.') { ext_len++; }
    if (filename[i] == '.') { break; }
  
    i--;
  }
  
  char* ext = malloc(ext_len);
  int prev_len = strlen(filename) - ext_len + 1;
  for (i = 0; i < ext_len; i++ ) {
    ext[i] = filename[prev_len + i];
  }
  
  ext[i] = '\0';
  
  return ext;
};

char* asset_file_location(char* filename) {

  int len = strlen(filename);
  int i = len;
  while( i > 0) {
    
    if (filename[i] != '/') { len--; }
    if (filename[i] == '/') { break; }
  
    i--;
  }
  i++;
  len++;
  
  char* main = malloc(len);
  memcpy(main, filename, len);
  main[len] = '\0';
  
  return main;
  
}