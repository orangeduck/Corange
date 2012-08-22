/**
*** :: Camera ::
***
***   Basic Camera Entity
***
**/

#ifndef camera_h
#define camera_h

#include "cengine.h"

typedef struct {
  vec3 position;
  vec3 target;
  float fov;
  float near_clip;
  float far_clip;
} camera;

camera* camera_new(vec3 position, vec3 target);
void camera_delete(camera* cam);

mat4 camera_view_matrix(camera* c);
mat4 camera_proj_matrix(camera* c, float aspect_ratio);
mat4 camera_view_proj_matrix(camera* c, float aspect_ratio);

#endif