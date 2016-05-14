/**
*** :: Camera ::
***
***   Basic Camera Entity
***
**/

#ifndef camera_h
#define camera_h

#include "cengine.h"
#include "cjoystick.h"

typedef struct {
  vec3 position;
  vec3 target;
  float fov;
  float near_clip;
  float far_clip;
} camera;

camera* camera_new();
void camera_delete(camera* cam);

vec3 camera_direction(camera* c);
mat4 camera_view_matrix(camera* c);
mat4 camera_proj_matrix(camera* c);
mat4 camera_view_proj_matrix(camera* c);

void camera_normalize_target(camera* c);
void camera_control_orbit(camera* c, SDL_Event e);
void camera_control_freecam(camera* c, float timestep);
void camera_control_joyorbit(camera* c, float timestep);

#endif