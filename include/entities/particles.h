#ifndef particles_h
#define particles_h

#include "cengine.h"
#include "cphysics.h"
#include "casset.h"

enum {
  MAX_EFFECTORS = 32,
};

typedef struct particles {
  
  vec3 position;
  mat4 rotation;
  vec3 scale;
  
  asset_hndl texture;
  GLuint blend_src;
  GLuint blend_dst;
  
  float lifetime;  
  int count;
  
  float* times;
  vec3*  scales;
  vec4*  colors;
  vec3*  positions;
  vec3*  velocities;
  
  GLuint vertex_buff;
  float* vertex_data;
  
  int effectors_num;
  void(*effectors[MAX_EFFECTORS])(struct particles*,float);
  
} particles;

typedef void (*particle_effector)(particles* p, float timestep);

void particle_effector_basic(particles* p, float timestep);
void particle_effector_time(particles* p, float timestep);
void particle_effector_movement(particles* p, float timestep);
void particle_effector_gravity(particles* p, float timestep);
void particle_effector_burst(particles* p, float timestep);

particles* partcicles_new();
void particles_delete(particles* p);

void particles_add_effector(particles* p, particle_effector pe);
void particles_allocate(particles* p, int count, float lifetime);

void particles_update(particles* p, float timestep);

#endif