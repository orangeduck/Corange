#ifndef randf_h
#define randf_h

#include "cengine.h"

/* Random float in range 0 to 1 */
float randf();
float randf_seed(float s);

/* Random float in range -1 to 1 */
float randf_n();
float randf_nseed(float s);

/* Random float in range 0 to s */
float randf_scale(float s);

/* Random float in range -s to s */
float randf_nscale(float s);

/* Random float in range s to e */
float randf_range(float s, float e);

vec2 randf_circle(float radius);

#endif