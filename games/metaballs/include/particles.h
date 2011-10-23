#ifndef particles_h
#define particles_h

#include "corange.h"

void particles_init();
void particles_finish();

void particles_update(float timestep);

int particles_count();

vector3 particle_position(int id);
vector3 particle_velocity(int id);


#endif