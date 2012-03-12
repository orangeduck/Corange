#include <math.h>
#include <stdlib.h>

#include "matrix.h"
#include "vector.h"

#include "camera.h"

#define DEFAULT_NEAR_CLIP 0.1
#define DEFAULT_FAR_CLIP 8192.0

#define DEFAULT_FOV 0.785398163

camera* camera_new(vector3 position, vector3 target) {

  camera* cam = malloc(sizeof(camera));
  
  cam->position = position;
  cam->target = target;
  cam->fov = DEFAULT_FOV;
  cam->near_clip = DEFAULT_NEAR_CLIP;
  cam->far_clip = DEFAULT_FAR_CLIP;
  
  return cam;
}

void camera_delete(camera* cam) {
  free(cam);
}

matrix_4x4 camera_view_matrix(camera* c) {
  return m44_view_look_at(c->position, c->target, v3(0.0f,1.0f,0.0f) );
};


matrix_4x4 camera_proj_matrix(camera* c, float aspect_ratio) {
  return m44_perspective(c->fov, c->near_clip, c->far_clip, aspect_ratio);
};


matrix_4x4 camera_view_proj_matrix(camera* c, float aspect_ratio) {
  
  matrix_4x4 view = camera_view_matrix(c);
  matrix_4x4 proj = camera_proj_matrix(c, aspect_ratio);
  return m44_mul_m44(view, proj);
  
};