#ifndef material_h
#define material_h

#include "dictionary.h"

typedef struct {

  char* name;
  dictionary* properties;
  
} material;

material* material_new();

material* mat_load_file(char* filename);

void material_delete(material* mat);


#endif