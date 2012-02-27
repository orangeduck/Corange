#ifndef terrain_h
#define terrain_h

#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "error.h"
#include "texture.h"

#define NUM_TERRAIN_BUFFERS 4

struct terrain_chunk {
  
  int id;
  int x, y, width, height;
  
  struct terrain_chunk* left;
  struct terrain_chunk* right;
  struct terrain_chunk* top;
  struct terrain_chunk* bottom;
  
  int num_indicies[NUM_TERRAIN_BUFFERS];
  GLuint index_buffers[NUM_TERRAIN_BUFFERS];
  
  int num_verts;
  GLuint vertex_buffer;
  
  texture* offset_map;
	
};

struct terrain_chunk;
typedef struct terrain_chunk terrain_chunk;

terrain_chunk* chnk_load_file(char* filename);
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
void terrain_delete(terrain* ter);

terrain_chunk* terrain_get_chunk(terrain* ter, int x, int y);

float terrain_height(terrain* ter, vector2 position);

#endif