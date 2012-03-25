#ifndef camera_h
#define camera_h

#include "vector.h"
#include "matrix.h"

typedef struct {
  
  vector3 position;
  vector3 target;
  
  float fov;
  
  float near_clip;
  float far_clip;
  
} camera;

camera* camera_new(vector3 position, vector3 target);
void camera_delete(camera* cam);

matrix_4x4 camera_view_matrix(camera* c);
matrix_4x4 camera_proj_matrix(camera* c, float aspect_ratio);
matrix_4x4 camera_view_proj_matrix(camera* c, float aspect_ratio);

#endif