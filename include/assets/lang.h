#ifndef lang_h
#define lang_h

#include "data/dict.h"
#include "casset.h"

typedef struct {
  dict* map;
} lang;

lang* lang_load_file(const char* filename);
void lang_delete(lang* t);
char* lang_get(lang* t, char* id);

void set_language(asset_hndl t);
char* S(char* id);

#endif