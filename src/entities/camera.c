#include "entities/camera.h"

#include "cgraphics.h"

camera* camera_new() {

  camera* c = malloc(sizeof(camera));
  
  c->position = vec3_new(10, 10, 10);
  c->target = vec3_zero();
  c->fov = 0.785398163;
  c->near_clip = 0.1;
  c->far_clip = 512.0;
  
  return c;
}

void camera_delete(camera* c) {
  free(c);
}

vec3 camera_direction(camera* c) {
  return vec3_normalize(vec3_sub(c->target, c->position));
}

mat4 camera_view_matrix(camera* c) {
  return mat4_view_look_at(c->position, c->target, vec3_new(0.0f,1.0f,0.0f) );
}

mat4 camera_proj_matrix(camera* c) {
  return mat4_perspective(c->fov, c->near_clip, c->far_clip, graphics_viewport_ratio());
}

mat4 camera_view_proj_matrix(camera* c) {
  mat4 view = camera_view_matrix(c);
  mat4 proj = camera_proj_matrix(c);
  return mat4_mul_mat4(view, proj);
}

void camera_normalize_target(camera* c) {
  c->target = vec3_add(c->position, vec3_normalize(vec3_sub(c->target, c->position)));
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
        c->position = mat3_mul_vec3(mat3_rotation_angle_axis(a2, axis), c->position );
      }
    break;
    
    case SDL_MOUSEWHEEL:
      c->position = vec3_add(c->position, vec3_mul(vec3_normalize(c->position), -e.wheel.y));
    break;

  }
  
  c->position = vec3_add(c->position, translation);
  c->target = vec3_add(c->target, translation);

}

void camera_control_freecam(camera* c, float timestep) {

  const Uint8* kbstate = SDL_GetKeyboardState(NULL);
  
  if (kbstate[SDL_SCANCODE_W] 
  ||  kbstate[SDL_SCANCODE_S] 
  ||  kbstate[SDL_SCANCODE_A] 
  ||  kbstate[SDL_SCANCODE_D]) {
    
    vec3 cam_dir = vec3_normalize(vec3_sub(c->target, c->position));
    vec3 side_dir = vec3_normalize(vec3_cross(cam_dir, vec3_new(0,1,0)));

    const float speed = 100 * timestep;
    
    if (kbstate[SDL_SCANCODE_W]) {
      c->position = vec3_add(c->position, vec3_mul(cam_dir, speed));
      c->target = vec3_add(c->target, vec3_mul(cam_dir, speed));
    }
    if (kbstate[SDL_SCANCODE_S]) {
      c->position = vec3_sub(c->position, vec3_mul(cam_dir, speed));
      c->target = vec3_sub(c->target, vec3_mul(cam_dir, speed));
    }
    if (kbstate[SDL_SCANCODE_D]) {
      c->position = vec3_add(c->position, vec3_mul(side_dir, speed));
      c->target = vec3_add(c->target, vec3_mul(side_dir, speed));
    }
    if (kbstate[SDL_SCANCODE_A]) {
      c->position = vec3_sub(c->position, vec3_mul(side_dir, speed));
      c->target = vec3_sub(c->target, vec3_mul(side_dir, speed));
    }
    
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
  c->position = mat3_mul_vec3(mat3_rotation_angle_axis(a2, axis), c->position );
  
  c->position = vec3_add(c->position, translation);
  c->target = vec3_add(c->target, translation);

}
