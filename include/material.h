#ifndef material_h
#define material_h

#include "dictionary.h"
#include "list.h"

typedef struct {

  char* name;
  list* keys;
  dictionary* properties;
  dictionary* types;
  
} material;

static int mat_type_program			= 0;
static int mat_type_texture 		= 3;
static int mat_type_string 			= 4;
static int mat_type_int 			= 5;
static int mat_type_float 			= 6;
static int mat_type_vector2 		= 7;
static int mat_type_vector3			= 8;
static int mat_type_vector4 		= 9;

material* material_new();
void material_delete(material* mat);

material* mat_load_file(char* filename);

void material_print(material* mat);

void material_parse_line(material* mat, char* line);

#endif