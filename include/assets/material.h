/**
*** :: Material ::
***
***   Material in system. Provides shader.
***   Also provides shader "uniform" values
***
**/

#ifndef material_h
#define material_h

#include "cengine.h"
#include "casset.h"

#include "assets/shader.h"

typedef union {
  int as_int;
  float as_float;
  vec2 as_vec2;
  vec3 as_vec3;
  vec4 as_vec4;
  asset_hndl as_asset;
} material_item;

static const int mat_item_int = 0;
static const int mat_item_float = 1;
static const int mat_item_vec2 = 2;
static const int mat_item_vec3 = 3;
static const int mat_item_vec4 = 4;
static const int mat_item_shader = 5;
static const int mat_item_texture = 6;

typedef struct {
  shader_program* program;
  int num_items;
  int* types;
  char** names;
  material_item* items;
} material_entry;


void material_entry_delete(material_entry* me);
material_item material_entry_item(material_entry* me, char* name);
bool material_entry_has_item(material_entry* me, char* name);
void material_entry_add_item(material_entry* me, char* name, int type, material_item mi);

typedef struct {
  int num_entries;
  material_entry** entries;
} material;

material* material_new();
void material_delete(material* m);

material* mat_load_file(char* filename);

material_entry* material_get_entry(material* m, int index);
material_entry* material_add_entry(material* m);

shader_program* material_first_program(material* m);

#endif