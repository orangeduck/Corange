#ifndef particles_h
#define particles_h

#include "cengine.h"
#include "casset.h"

#include "assets/effect.h"

#include "entities/camera.h"

typedef struct particles {
  
  vec3 position;
  quat rotation;
  vec3 scale;
  
  asset_hndl effect;
  
  float rate;
  
  int count;
  bool*  actives;
  float* seeds;
  float* times;
  float* rotations;
  vec3*  scales;
  vec4*  colors;
  vec3*  positions;
  vec3*  velocities;
  
  GLuint vertex_buff;
  float* vertex_data;
  
} particles;

particles* particles_new();
void particles_delete(particles* p);
void particles_set_effect(particles* p, asset_hndl effect);
void particles_update(particles* p, float timestep, camera* cam);

#endif