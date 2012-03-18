#ifndef particles_h
#define particles_h

#include "corange.h"

#include "kernel.h"

void particles_init();
void particles_finish();

void particles_update(float timestep);

int particles_count();

void particles_reset();

kernel_memory particle_positions_memory();

void particle_positions(vector4* out);

GLuint particle_positions_buffer();
GLuint particle_velocities_buffer();

#endif