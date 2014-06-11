#include "assets/lang.h"

static int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDL_RWread(file, &c, 1, 1);
    
    if (status == -1) return -1;
    if (i == buffersize-1) return -1;
    if (status == 0) break;
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    buffer[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

lang* lang_load_file(const char* filename) {
  
  lang* t = malloc(sizeof(lang));
  t->map = dict_new(512);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    char id[1024];
    char text[1024];
    if (sscanf(line, "%s %[^\r\n]", id, text) == 2) {
      
      /* Replace newlines */
      for(int i = 0; i < strlen(text); i++) {
        if (text[i] == '\\' && text[i+1] == 'n') {
          text[i] = ' ';
          text[i+1] = '\n';
        }
      }
      
      char* text_cpy = malloc(strlen(text) + 1); strcpy(text_cpy, text);
      dict_set(t->map, id, text_cpy);
    }
  
  }
  
  SDL_RWclose(file);
  
  return t;
  
}

void lang_delete(lang* t) {
  dict_map(t->map, free);
  dict_delete(t->map);
  free(t);
}

char* lang_get(lang* t, char* id) {
  return dict_get(t->map, id);
}

static asset_hndl curr_lang;

void set_language(asset_hndl t) {
  curr_lang = t;
}

char* S(char* id) {
  if (!asset_hndl_isnull(&curr_lang)) {
    return lang_get(asset_hndl_ptr(&curr_lang), id);
  } else {
    error("Current Language hasn't been set!");
    return NULL;
  }
}