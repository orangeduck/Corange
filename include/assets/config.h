#ifndef config_h
#define config_h

#include "cengine.h"
#include "casset.h"
#include "data/dict.h"

typedef struct {
  dict* entries;
} config;

config* cfg_load_file(const char* filename);
void cfg_save_file(config* c, const char* filename);
void config_delete(config* c);

char* config_string(config* c, char* key);
int   config_int(config* c, char* key);
float config_float(config* c, char* key);
bool  config_bool(config* c, char* key);

void config_set_string(config* c, char* key, char* val);
void config_set_int(config* c, char* key, int val);
void config_set_float(config* c, char* key, float val);
void config_set_bool(config* c, char* key, bool val);

asset_hndl option_graphics_asset(config* c, char* key, asset_hndl high, asset_hndl medium, asset_hndl low);
int option_graphics_int(config* c, char* key, int high, int medium, int low);
float option_graphics_float(config* c, char* key, float high, float medium, float low);

#endif