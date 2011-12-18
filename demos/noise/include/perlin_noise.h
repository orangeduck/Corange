#ifndef perlin_noise_h
#define perlin_noise_h

#include "image.h"
#include "vector.h"

float perlin_noise2D(vector2 v);

image* perlin_noise_generate(int x_size, int y_size, int octaves);

#endif