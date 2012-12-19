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

#endif