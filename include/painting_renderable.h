#ifndef painting_renderable_h
#define painting_renderable_h

#include "renderable.h"

#include "texture.h"


/* Particle VBO is structured in this way */

/* vec3 ParticlePosition | vec2 ParticleUVs | vec3 FacePosition | vec3 FaceNormal */

typedef struct {
  
  renderable* renderable;  
  
  GLuint position_vbo;
  GLuint uvs_vbo;
  GLuint face_position_vbo;
  GLuint face_normal_vbo; 
  
  GLuint* index_vbos;
  int num_index_vbos;
  
  int num_particles;
  
  float density;
  float particle_size;
  float particle_min_size;
  float particle_max_size;
  
  texture* brush;

} painting_renderable;

painting_renderable* painting_renderable_new(char* name);
void painting_renderable_delete(painting_renderable* pr);

void painting_renderable_add_model(painting_renderable* r, model* m);

#endif