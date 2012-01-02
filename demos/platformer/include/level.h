#ifndef level_h
#define level_h

#include "corange.h"

typedef struct {
  
  int num_tiles;
  GLuint positions_buffer;
  GLuint texcoords_buffer;
  material* mat;
  
} tile_set;

typedef struct {

  int num_tile_sets;
  int* tile_map;
  tile_set* tile_sets;

} level;

level* level_load_file(char* filename);

void level_delete(level* tl);

void level_render(level* tl, vector2 camera_position);

#endif
