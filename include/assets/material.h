#ifndef material_h
#define material_h

#include "dictionary.h"
#include "list.h"

#define mat_type_program 1
#define mat_type_shader 2
#define mat_type_texture 3
#define mat_type_string 4
#define mat_type_int 5
#define mat_type_float 6
#define mat_type_vector2 7
#define mat_type_vector3 8
#define mat_type_vector4 9

typedef struct {

  char* name;
  list* keys;
  dictionary* properties;
  dictionary* types;
  
} material;

material* material_new();
void material_delete(material* mat);

material* mat_load_file(char* filename);

void material_print(material* mat);

void* material_get_property(material* mat, char* name);
int material_get_type(material* mat, char* name);
void material_set_property(material* mat, char* name, void* value, int type);

typedef struct {

  int num_materials;
  material** materials;
  
} multi_material;

multi_material* mmat_load_file(char* filename);
void multi_material_delete(multi_material* mmat);

#endif