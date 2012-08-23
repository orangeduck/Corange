#include <math.h>

#include "sea.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

void sea_init() {
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Sea");

  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(50.0, 50.0, 50.0);
  cam->target = vec3_new(0, 5, 0);
  cam->near_clip = 0.1;
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_spot);
  sun->position = vec3_new(20,23,16);
  sun->ambient_color = vec3_new(0.5, 0.5, 0.5);
  sun->diffuse_color = vec3_new(1.0,  0.894, 0.811);
  sun->specular_color = vec3_mul(vec3_new(1.0,  0.894, 0.811), 4);
  sun->power = 5;
  
  light* backlight = entity_new("backlight", light);
  light_set_type(backlight, light_type_point);
  backlight->position = vec3_new(-22,10,-13);
  backlight->ambient_color = vec3_new(0.2, 0.2, 0.2);
  backlight->diffuse_color = vec3_new(0.729, 0.729, 1.0);
  backlight->specular_color = vec3_mul(vec3_new(0.729, 0.729, 1.0), 1);
  backlight->power = 2;
  
  shadow_mapper_init(sun);  
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_shadow_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  forward_renderer_add_light(sun);
  forward_renderer_add_light(backlight);
  
  folder_load(P("./resources/"));
  
  renderable* r_seaplane = asset_get(P("./resources/seaplane.obj"));
  r_seaplane->material = asset_hndl_new(P("./resources/seaplane.mat"));
  
  static_object* s_seaplane = entity_new("seaplane", static_object);
  s_seaplane->renderable = asset_hndl_new_ptr(r_seaplane);
  s_seaplane->scale = vec3_new(3,1,3);
  
  renderable* r_skydome = asset_get(P("./resources/skydome.obj"));
  r_skydome->material = asset_hndl_new_load(P("$CORANGE/shaders/skydome.mat"));
  
  static_object* skydome = entity_new("skydome", static_object);
  skydome->renderable = asset_hndl_new_ptr(r_skydome);
  skydome->position = vec3_new(0, -512, 0);
  skydome->scale = vec3_new(1024, 1024, 1024);
  
  folder_load(P("./resources/corvette/"));
  
  renderable* r_corvette = asset_get(P("./resources/corvette/corvette.obj"));
  r_corvette->material = asset_hndl_new_load(P("./resources/corvette/corvette.mat"));
  
  static_object* s_corvette = entity_new("corvette", static_object);
  s_corvette->renderable = asset_hndl_new_ptr(r_corvette);
  //s_corvette->collision_body = collision_body_new_mesh(asset_get("./resources/corvette/corvette.col"));
  s_corvette->scale = vec3_new(1.5, 1.5, 1.5);
  s_corvette->position = vec3_new(0, 0.5, 0);
  
  static_object* center_sphere = entity_new("center_sphere", static_object);
  center_sphere->position = vec3_new(0, 5, 0);
  //center_sphere->renderable = asset_get("./resources/ball.obj");
  //center_sphere->collision_body = collision_body_new_sphere(sphere_new(v3_zero(), 1.0f));
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  corvette->position.y = (sin(wave_time) + 1) / 2;
  corvette->rotation = quaternion_pitch(sin(wave_time * 1.123) / 50);
  corvette->rotation = quaternion_mul(corvette->rotation, quaternion_yaw(sin(wave_time * 1.254) / 25));
  corvette->rotation = quaternion_mul(corvette->rotation, quaternion_roll(sin(wave_time * 1.355) / 100));
  
  static_object* center_sphere = entity_get("center_sphere");
  
  physics_object* balls[100];
  int num_balls;
  entities_get(balls, &num_balls, physics_object);
  for(int i = 0; i < num_balls; i++) {
    physics_object_collide_static(balls[i], center_sphere, frame_time());
    physics_object_collide_static(balls[i], corvette, frame_time());
    physics_object_update(balls[i], frame_time());
  }
  
  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  
  if(keystate & SDL_BUTTON(3)){
    sun->position.x += (float)mouse_y / 2;
    sun->position.z -= (float)mouse_x / 2;
  }

  mouse_x = 0;
  mouse_y = 0;
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
}

void sea_render() {

  static_object* s_seaplane = entity_get("seaplane");
  static_object* s_skydome = entity_get("skydome");
  static_object* s_corvette = entity_get("corvette");
  light* sun = entity_get("sun");
  light* backlight = entity_get("backlight");
  
  shadow_mapper_begin();
  shadow_mapper_render_static(s_corvette);
  shadow_mapper_end();
  
  forward_renderer_begin();
  
  forward_renderer_render_static(s_skydome);
  forward_renderer_render_static(s_seaplane);
  forward_renderer_render_static(s_corvette);
  
  
  physics_object* balls[100];
  int num_balls;
  entities_get(balls, &num_balls, physics_object);
  for(int i = 0; i < num_balls; i++) {
    //forward_renderer_render_physics(balls[i]);
  }
  
  //static_object* center_sphere = entity_get("center_sphere");
  //forward_renderer_render_static(center_sphere);
  
  forward_renderer_render_light(sun);
  forward_renderer_render_light(backlight);
  
  forward_renderer_end();
  
}

static int ball_count = 0;
void sea_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  camera_control_orbit(cam, event);
  
  switch(event.type){
  case SDL_KEYUP:
  
    if (event.key.keysym.sym == SDLK_SPACE) {
      
      //char ball_name[20];
      //sprintf(ball_name, "ball_%i", ball_count);
      //ball_count++;
      
      //physics_object* ball = entity_new(ball_name, physics_object);
      //ball->renderable = asset_get(P("./resources/ball.obj"));
      //ball->collision_body = collision_body_new_sphere(sphere_new(v3_zero(), 1));
      //ball->position = cam->position;
      //ball->scale = v3(0.5, 0.5, 0.5);
      //ball->velocity = v3_mul(v3_normalize(v3_sub(cam->target, cam->position)), 75);
      
    }
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }

}

void sea_finish() {
  forward_renderer_finish();
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  sea_init();
  
  int running = 1;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      sea_event(event);
      ui_event(event);
    }
    
    sea_update();
    ui_update();
    
    sea_render();
    ui_render();
    
    SDL_GL_SwapBuffers();
    
    frame_end();
  }
  
  sea_finish();
  
  corange_finish();
  
  return 0;
  
}
