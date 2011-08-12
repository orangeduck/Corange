#ifndef perlin_noise_h
#define perlin_noise_h

#include "texture.h"
#include "vector.h"

float perlin_noise2D(vector2 v);

texture* perlin_noise_generate_texture2D(int x_size, int y_size, int octaves);

#endif