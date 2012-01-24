#ifndef material_h
#define material_h

#include "dictionary.h"
#include "list.h"

#define mat_type_program 0
#define mat_type_texture 1
#define mat_type_string 2
#define mat_type_int 3
#define mat_type_float 4
#define mat_type_vector2 5
#define mat_type_vector3 6
#define mat_type_vector4 7

#define blend_one 0
#define blend_zero 1
#define blend_src_alpha 2
#define blend_dst_alpha 3
#define blend_one_minus_src_alpha 4
#define blend_one_minus_dst_alpha 5
#define blend_src_color 6
#define blend_dst_color 7
#define blend_one_minus_src_color 8
#define blend_one_minus_dst_color 9

typedef struct {

  char* name;
  list* keys;
  dictionary* properties;
  dictionary* types;
  
  bool use_blending;
  int src_blend_func;
  int dst_blend_func;
  
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