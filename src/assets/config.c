
#include "assets/config.h"

config* cfg_load_file(const char* filename) {

  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  config* c = malloc(sizeof(config));
  c->entries = dict_new(512);
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (line[0] == '#') continue;
    if (line[0] == '\r') continue;
    if (line[0] == '\n') continue;
    
    char key[1024];
    char val[1024];
    
    if (sscanf(line, "%[^ \r\n=] = %[^ \r\n=]", key, val) == 2) {
      char* val_cpy = malloc(strlen(val) + 1);
      strcpy(val_cpy, val);
      dict_set(c->entries, key, val_cpy);
    }
    
  }
  
  SDL_RWclose(file);
  
  return c;
  
}

void config_delete(config* c) {
  dict_map(c->entries, free);
  dict_delete(c->entries);
  free(c);
}

char* config_string(config* c, char* key) {
  char* out = dict_get(c->entries, key);
  if (out) {
    return out;
  } else {
    error("key '%s' not in config file!", key);
    return NULL;
  }
}

int config_int(config* c, char* key) {
  char* out = dict_get(c->entries, key);
  if (out) {
    return atoi(out);
  } else {
    error("key '%s' not in config file!", key);
    return 0;
  }
}

float config_float(config* c, char* key) {
  char* out = dict_get(c->entries, key);
  if (out) {
    return atof(out);
  } else {
    error("key '%s' not in config file!", key);
    return 0.0;
  }
}

bool config_bool(config* c, char* key) {
  char* out = dict_get(c->entries, key);
  if (out) {
    if (strstr(out, "false")) { return false; }
    if (strstr(out, "true"))  { return true; }
    return (bool)atoi(out);
  } else {
    error("key '%s' not in config file!", key);
    return false;
  }  
}