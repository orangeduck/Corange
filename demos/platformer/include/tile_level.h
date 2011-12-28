#ifndef tile_level_h
#define tile_level_h

#include "corange.h"

typedef struct {

  GLuint positions_buffer;
  GLuint texcoords_buffer;
  material* mat;
  
} tile_set;

typedef struct {

  int num_tile_sets;
  tile_set* tile_sets;

} tile_level;

tile_level* level_load_file(char* filename);

void tile_level_delete(tile_level* tl);

#endif
