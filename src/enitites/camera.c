#include "entities/camera.h"

#define DEFAULT_NEAR_CLIP 0.1
#define DEFAULT_FAR_CLIP 8192.0

#define DEFAULT_FOV 0.785398163

camera* camera_new(vec3 position, vec3 target) {

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

mat4 camera_view_matrix(camera* c) {
  return mat4_view_look_at(c->position, c->target, vec3_new(0.0f,1.0f,0.0f) );
}

mat4 camera_proj_matrix(camera* c, float aspect_ratio) {
  return mat4_perspective(c->fov, c->near_clip, c->far_clip, aspect_ratio);
}

mat4 camera_view_proj_matrix(camera* c, float aspect_ratio) {
  mat4 view = camera_view_matrix(c);
  mat4 proj = camera_proj_matrix(c, aspect_ratio);
  return mat4_mul_mat4(view, proj);
}

void camera_control_orbit(camera* c, SDL_Event e) {
  
  float a1, a2;
  vec3 axis;
  
  switch(e.type) {
    case SDL_MOUSEMOTION:
      if (e.motion.state & SDL_BUTTON(1)) {
        a1 = e.motion.xrel * -0.005;
        a2 = e.motion.yrel * 0.005;
        c->position = mat3_mul_vec3(mat3_rotation_y( a1 ), c->position );
        axis = vec3_normalize(vec3_cross( vec3_sub(c->position, c->target) , vec3_new(0,1,0) ));
        c->position = mat3_mul_vec3(mat3_rotation_axis_angle(axis, a2 ), c->position );
      }
    break;
    
    case SDL_MOUSEBUTTONDOWN:
      if (e.button.button == SDL_BUTTON_WHEELUP) {
        c->position = vec3_sub(c->position, vec3_normalize(c->position));
      }
      if (e.button.button == SDL_BUTTON_WHEELDOWN) {
        c->position = vec3_add(c->position, vec3_normalize(c->position));
      }
    break;
  }

}
