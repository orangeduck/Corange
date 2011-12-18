#ifndef wolf_renderable_h
#define wolf_renderable_h

#include "corange.h"

typedef struct {
  
  renderable* renderable;
  
  int num_particles;
  
  GLuint brush_id_vbo;
  GLuint position_vbo;
  GLuint direction_vbo;
  
  texture* brush_texture;
  int num_brushes;
  vector2 brush_scale;
  
} wolf_renderable;

wolf_renderable* wolf_renderable_new(char* name, model* m, texture* brush_texture, int num_brushes, vector2 brush_scale, float density);

void wolf_renderable_delete(wolf_renderable* pr);

#endif