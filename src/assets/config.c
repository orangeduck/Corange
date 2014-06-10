
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

void cfg_save_file(config* c, const char* filename) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "w");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  void write_entry(void* x) {
    char* val = x;
    char* key = dict_find(c->entries, val);
  
    SDL_RWwrite(file, key, strlen(key), 1);
    SDL_RWwrite(file, " = ", 3, 1);
    SDL_RWwrite(file, val, strlen(val), 1);
    SDL_RWwrite(file, "\n", 1, 1);
  }
  
  dict_map(c->entries, write_entry);
  
  SDL_RWclose(file);
  
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

void config_set_string(config* c, char* key, char* val) {
  
  free(dict_get(c->entries, key));

  char* item = malloc(strlen(val) + 1);
  strcpy(item, val);  
  dict_set(c->entries, key, item);
  
}

void config_set_int(config* c, char* key, int val) {

  free(dict_get(c->entries, key));

  char* item = malloc(val / 10 + 10);
  sprintf(item, "%i", val);
  dict_set(c->entries, key, item);

}

void config_set_float(config* c, char* key, float val) {

  free(dict_get(c->entries, key));

  /* http://stackoverflow.com/questions/1701055/what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value */
  char* item = malloc(30);
  sprintf(item, "%f", val);
  dict_set(c->entries, key, item);

}

void config_set_bool(config* c, char* key, bool val) {

  free(dict_get(c->entries, key));

  char* item = malloc(6);
  strcpy(item, val ? "true" : "false");
  dict_set(c->entries, key, item);

}

asset_hndl option_graphics_asset(config* c, char* key, asset_hndl high, asset_hndl medium, asset_hndl low) {
  int val = config_int(c, key);
  if (val == 2) { return high; }
  if (val == 1) { return medium; }
  return low;
}

int option_graphics_int(config* c, char* key, int high, int medium, int low) {
  int val = config_int(c, key);
  if (val == 2) { return high; }
  if (val == 1) { return medium; }
  return low;
}

float option_graphics_float(config* c, char* key, float high, float medium, float low) {
  int val = config_int(c, key);
  if (val == 2) { return high; }
  if (val == 1) { return medium; }
  return low;
}
