#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "game.h"

static char game_name_str[128];

static char* game_dll_location;

static char* game_init_func_name;
static char* game_update_func_name;
static char* game_render_func_name;
static char* game_finish_func_name;
static char* game_event_func_name; 

static HMODULE game_library;

static int game_loaded = 0;

static int game_has_init = 0;
static int game_has_update = 0;
static int game_has_render = 0;
static int game_has_finish = 0;
static int game_has_event = 0;

static void (*game_init_func)();
static void (*game_update_func)();
static void (*game_render_func)();
static void (*game_finish_func)();
static void (*game_event_func)(SDL_Event);

char* game_name() {
  return game_name_str;
}

void game_load(char* name) {
  
  strcpy(game_name_str, name);
  
  if(game_loaded) {
    game_unload();
  }
  
  game_dll_location = malloc( strlen("./Games/") + 
                              strlen(name) + 
                              strlen("/") + 
                              strlen(name) + 
                              strlen(".dll") + 1 );
  
  strcpy(game_dll_location, "./Games/");
  strcat(game_dll_location, name);
  strcat(game_dll_location, "/");
  strcat(game_dll_location, name);
  strcat(game_dll_location, ".dll");
    
  game_library = LoadLibrary(game_dll_location);
  if( game_library == NULL) {
     
     printf("Could not load game '%s'!\n", name);
     
  } else {
    
    printf("Loaded Game '%s'\n", name);
    game_loaded = 1;
    
    game_init_func_name = malloc(strlen(name) + strlen("_init") + 1);
    game_update_func_name = malloc(strlen(name) + strlen("_update") + 1);
    game_render_func_name = malloc(strlen(name) + strlen("_render") + 1);
    game_finish_func_name = malloc(strlen(name) + strlen("_finish") + 1);
    game_event_func_name = malloc(strlen(name) + strlen("_event") + 1);
    
    strcpy(game_init_func_name, name); strcat(game_init_func_name, "_init");
    strcpy(game_update_func_name, name); strcat(game_update_func_name, "_update");
    strcpy(game_render_func_name, name); strcat(game_render_func_name, "_render");
    strcpy(game_finish_func_name, name); strcat(game_finish_func_name, "_finish");
    strcpy(game_event_func_name, name); strcat(game_event_func_name, "_event");
    
    game_init_func = (void(*)(void))GetProcAddress(game_library, game_init_func_name);
    if ( game_init_func == NULL ) {
      printf("Couldn't find init function '%s' for game '%s'\n", game_init_func_name, name);
    } else {
      game_has_init = 1;
    }
    
    game_update_func = (void(*)(void))GetProcAddress(game_library, game_update_func_name);
    if ( game_update_func == NULL ) {
      printf("Couldn't find update function '%s' for game '%s'\n", game_update_func_name, name);
    } else {
      game_has_update = 1;
    }
    
    game_render_func = (void(*)(void))GetProcAddress(game_library, game_render_func_name);
    if ( game_render_func == NULL ) {
      printf("Couldn't find render function '%s' for game '%s'\n", game_render_func_name, name);
    } else {
      game_has_render = 1;
    }
    
    
    game_finish_func = (void(*)(void))GetProcAddress(game_library, game_finish_func_name);
    if ( game_finish_func == NULL ) {
      printf("Couldn't find finish function '%s' for game '%s'\n", game_finish_func_name, name);
    } else {
      game_has_finish = 1;
    }
    
    game_event_func = (void(*)(SDL_Event))GetProcAddress(game_library, game_event_func_name);
    if ( game_event_func == NULL ) {
      printf("Couldn't find event function '%s' for game '%s'\n", game_event_func_name, name);
    } else {
      game_has_event = 1;
    }
  
  }
  
}

void game_unload() {
  
  FreeLibrary(game_library);
  free(game_dll_location);
  
  free(game_init_func_name);
  free(game_update_func_name);
  free(game_render_func_name);
  free(game_finish_func_name);
  free(game_event_func_name);
  
  game_loaded = 0;

  game_has_init = 0;
  game_has_update = 0;
  game_has_render = 0;
  game_has_finish = 0;
  game_has_event = 0;
    
}

void game_init() {
  
  if(game_loaded && game_has_init) {
    game_init_func();
  }
}

void game_update() {

  if(game_loaded && game_has_update) {
    game_update_func();
  }
}

void game_event(SDL_Event event) {

  if(game_loaded && game_has_event) {
    game_event_func(event);
  }
}

void game_render() {

  if(game_loaded && game_has_render) {
    game_render_func();
  }
}

void game_finish() {

  if(game_loaded && game_has_finish) {
    game_finish_func();
  }
}