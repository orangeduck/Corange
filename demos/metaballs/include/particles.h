#ifndef metaball_particles_h
#define metaball_particles_h

#include "corange.h"

#include "kernel.h"

void metaball_particles_init();
void metaball_particles_finish();

void metaball_particles_update(float timestep);

int metaball_particles_count();

void metaball_particles_reset();

kernel_memory metaball_particle_positions_memory();

GLuint metaball_particle_positions_buffer();
GLuint metaball_particle_velocities_buffer();

void metaball_particles_render();

#endif