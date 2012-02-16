#ifndef marching_cubes_h
#define marching_cubes_h

#include "corange.h"

#include "kernel.h"

void marching_cubes_init();
void marching_cubes_finish();

void marching_cubes_clear();
void marching_cubes_point(int x, int y, int z, float value);
void marching_cubes_metaball(vector3 pos, int id);

void marching_cubes_metaball_data(kernel_memory positions, int num_metaballs);

void marching_cubes_update();
void marching_cubes_render(bool wireframe, vector3 camera_position, vector3 light_position);

void marching_cubes_render_shadows(light* l);

#endif