#ifndef particles_h
#define particles_h

#include "corange.h"

void particles_init();
void particles_finish();

void particles_update(float timestep);

int particles_count();

vector4 particle_position(int id);
vector4 particle_velocity(int id);

GLuint particle_positions_buffer();
GLuint particle_velocities_buffer();

#endif