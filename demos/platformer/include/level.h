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

void level_delete(level* l);

void level_render(level* l, vector2 camera_position);

int level_tile_at(level* l, vector2 position);
vector2 level_tile_position(level* l, int x, int y);

#endif
