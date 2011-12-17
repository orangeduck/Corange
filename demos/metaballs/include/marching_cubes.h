#ifndef marching_cubes_h
#define marching_cubes_h

#include "corange.h"

void marching_cubes_init();
void marching_cubes_finish();

void marching_cubes_clear();
void marching_cubes_point(int x, int y, int z, float value);
void marching_cubes_metaball(float x, float y, float z, float size);

void marching_cubes_update();
void marching_cubes_render();

#endif