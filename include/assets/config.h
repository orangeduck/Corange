#ifndef config_h
#define config_h

#include "cengine.h"
#include "casset.h"
#include "data/dict.h"

typedef struct {
  dict* entries;
} config;

config* cfg_load_file(const char* filename);
void config_delete(config* c);

char* config_string(config* c, char* key);
int config_int(config* c, char* key);
float config_float(config* c, char* key);
bool config_bool(config* c, char* key);

asset_hndl option_graphics_asset(config* c, char* key, asset_hndl high, asset_hndl medium, asset_hndl low);
int option_graphics_int(config* c, char* key, int high, int medium, int low);

#endif