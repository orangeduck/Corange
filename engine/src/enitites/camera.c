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
  
  float a1 = 0;
  float a2 = 0;
  vec3 axis;
  
  vec3 translation = c->target;
  c->position = vec3_sub(c->position, translation);
  c->target = vec3_sub(c->target, translation);
  
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
  
  c->position = vec3_add(c->position, translation);
  c->target = vec3_add(c->target, translation);

}

void camera_control_freecam(camera* c, float timestep) {

  Uint8* kbstate = SDL_GetKeyState(NULL);
  
  if (kbstate[SDLK_w] || kbstate[SDLK_s]) {
    
    vec3 cam_dir = vec3_normalize(vec3_sub(c->target, c->position));
    
    const float speed = 100 * timestep;
    
    if (kbstate[SDLK_w]) {
      c->position = vec3_add(c->position, vec3_mul(cam_dir, speed));
    }
    if (kbstate[SDLK_s]) {
      c->position = vec3_sub(c->position, vec3_mul(cam_dir, speed));
    }
    
    c->target = vec3_add(c->position, cam_dir);
  }
  
  int mouse_x, mouse_y;
  Uint8 mstate = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
  if (mstate & SDL_BUTTON(1)) {
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    vec3 cam_dir = vec3_normalize(vec3_sub(c->target, c->position));
    
    cam_dir.y += -a2;
    vec3 side_dir = vec3_normalize(vec3_cross(cam_dir, vec3_new(0,1,0)));
    cam_dir = vec3_add(cam_dir, vec3_mul(side_dir, -a1));
    cam_dir = vec3_normalize(cam_dir);
    
    c->target = vec3_add(c->position, cam_dir);
  }
}

void camera_control_joyorbit(camera* c, float timestep) {
  
  if (joystick_count() == 0) return;
  
  SDL_Joystick* mainstick = joystick_get(0);
  int x_move = SDL_JoystickGetAxis(mainstick, 0);
  int y_move = SDL_JoystickGetAxis(mainstick, 1);
  
  /* Dead Zone */
  if (abs(x_move) < 10000) { x_move = 0; };
  if (abs(y_move) < 10000) { y_move = 0; };
  
  float a1 = (x_move / 32768.0) * -0.05;
  float a2 = (y_move / 32768.0) * 0.05;
  
  vec3 translation = c->target;
  c->position = vec3_sub(c->position, translation);
  c->target = vec3_sub(c->target, translation);
  
  c->position = mat3_mul_vec3(mat3_rotation_y( a1 ), c->position );
  vec3 axis = vec3_normalize(vec3_cross( vec3_sub(c->position, c->target) , vec3_new(0,1,0) ));
  c->position = mat3_mul_vec3(mat3_rotation_axis_angle(axis, a2 ), c->position );
  
  c->position = vec3_add(c->position, translation);
  c->target = vec3_add(c->target, translation);

}
