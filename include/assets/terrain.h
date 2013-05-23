/**
*** :: Terrain ::
***
***   - WIP -
***
***   Accellerated terrain loaded from heightmap
***   splits into chunks for faster rendering
***   builds dynamic LODs into several index buffers
***
**/

#ifndef terrain_h
#define terrain_h

#include "cengine.h"

#include "assets/cmesh.h"

#define NUM_TERRAIN_SUBDIVISIONS 0
#define NUM_TERRAIN_BUFFERS 7

struct terrain_chunk {
  
  int id;
  int x, y, width, height;
  
  sphere bound;
  
  struct terrain_chunk* left;
  struct terrain_chunk* right;
  struct terrain_chunk* top;
  struct terrain_chunk* bottom;
  
  cmesh* colmesh;
  
  int num_verts;
  GLuint vertex_buffer;
  
  int num_indicies[NUM_TERRAIN_BUFFERS];
  GLuint index_buffers[NUM_TERRAIN_BUFFERS];
  
};

struct terrain_chunk;
typedef struct terrain_chunk terrain_chunk;

void terrain_chunk_delete(terrain_chunk* tc);

typedef struct {
  
  int width;
  int height;
  float* heightmap;

  int chunk_width;
  int chunk_height;
  
  int num_chunks;
  int num_cols;
  int num_rows;
  
  terrain_chunk** chunks;

} terrain;

terrain* raw_load_file(char* filename);
void raw_save_file(terrain* ter, char* filename);
void terrain_delete(terrain* ter);

terrain_chunk* terrain_get_chunk(terrain* ter, int x, int y);
void terrain_reload_chunk(terrain* ter, int i);

mat3  terrain_tbn(terrain* ter, vec2 position);
mat3  terrain_axis(terrain* ter, vec2 position);
float terrain_height(terrain* ter, vec2 position);
vec3  terrain_normal(terrain* ter, vec2 position);

#endif