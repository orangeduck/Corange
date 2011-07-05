#ifndef painting_renderable_h
#define painting_renderable_h

#include "renderable.h"

#include "texture.h"
#include "vector.h"


/* Particle VBO is structured in this way */

/* vec3 ParticlePosition | vec2 ParticleUVs | vec3 FacePosition | vec3 FaceNormal */

typedef struct {
  
  renderable* renderable;  
  
  GLuint position_vbo;
  GLuint uvs_vbo;
  GLuint face_position_vbo;
  GLuint face_normal_vbo;
  GLuint face_tangent_vbo;
  
  GLuint* index_vbos;
  int num_index_vbos;
  
  int num_particles;
  
  float density;
  
  texture* brush;
  vector2 brush_size;

  int alignment;
  
} painting_renderable;

static int align_x_axis = 0;
static int align_y_axis = 1;
static int align_auto = 2;

painting_renderable* painting_renderable_new(char* name, float density, vector2 brush_size, texture* brush, int alignment);
void painting_renderable_delete(painting_renderable* pr);

void painting_renderable_add_model(painting_renderable* r, model* m);

#endif