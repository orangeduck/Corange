#include <math.h>

#include "sea.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

void sea_init() {
  
  viewport_set_dimensions(1280, 720);

  camera* cam = entity_new("camera", camera);
  cam->position = v3(50.0, 50.0, 50.0);
  cam->target = v3(0, 5, 0);
  cam->near_clip = 0.1;
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_spot);
  sun->position = v3(20,23,16);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(1.0,  0.894, 0.811);
  sun->specular_color = v3_mul(v3(1.0,  0.894, 0.811), 4);
  sun->power = 5;
  
  light* backlight = entity_new("backlight", light);
  light_set_type(backlight, light_type_point);
  backlight->position = v3(-22,10,-13);
  backlight->ambient_color = v3(0.2, 0.2, 0.2);
  backlight->diffuse_color = v3(0.729, 0.729, 1.0);
  backlight->specular_color = v3_mul(v3(0.729, 0.729, 1.0), 1);
  backlight->power = 2;
  
  shadow_mapper_init(sun);  
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_shadow_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  forward_renderer_add_light(sun);
  forward_renderer_add_light(backlight);
  
  load_folder("./resources/");
   
  texture* noise1 = asset_get("./resources/noise1.dds");
  texture* noise2 = asset_get("./resources/noise2.dds");
  texture* noise3 = asset_get("./resources/noise3.dds");
  texture* noise4 = asset_get("./resources/noise4.dds");
  texture* noise5 = asset_get("./resources/noise5.dds");
  
  texture* skydome = asset_get("./resources/skybox_cloud_10.dds");

  texture* water_calm = asset_get("./resources/water_calm.dds");
  texture* water_foam = asset_get("./resources/water_foam.dds");
  
  material* seaplane_mat = asset_get("./resources/seaplane.mat");
  
  material_set_property(seaplane_mat, "tex_noise1", noise1, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise2", noise2, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise3", noise3, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise4", noise4, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise5", noise5, mat_type_texture);
  
  material_set_property(seaplane_mat, "tex_skybox", skydome, mat_type_texture);

  material_set_property(seaplane_mat, "tex_calm_water", water_calm, mat_type_texture);
  material_set_property(seaplane_mat, "tex_foam_water", water_foam, mat_type_texture);
  
  renderable* r_seaplane = asset_get("./resources/seaplane.obj");
  renderable_set_material(r_seaplane, seaplane_mat);
  entity_add("seaplane", static_object, static_object_new(r_seaplane));
  
  load_folder("./resources/corvette/");
  
  renderable* r_corvette = asset_get("./resources/corvette/corvette.obj");
  multi_material* m_corvette = asset_get("./resources/corvette/corvette.mmat");
  renderable_set_multi_material(r_corvette, m_corvette);
  static_object* s_corvette = static_object_new(r_corvette);
  s_corvette->collision_body = collision_body_new_mesh(asset_get("./resources/corvette/corvette.col"));
  s_corvette->scale = v3(1.5, 1.5, 1.5);
  s_corvette->position = v3(0, 0.5, 0);
  entity_add("corvette", static_object, s_corvette);
  
  static_object* center_sphere = entity_new("center_sphere", static_object);
  center_sphere->position = v3(0, 5, 0);
  center_sphere->renderable = asset_get("./resources/ball.obj");
  center_sphere->collision_body = collision_body_new_sphere(sphere_new(v3_zero(), 1.0f));
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, v2(10,10));
  ui_button_resize(framerate, v2(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  //corvette->position.y = (sin(wave_time) + 1) / 2;
  //corvette->rotation = v4_quaternion_pitch(sin(wave_time * 1.123) / 50);
  //corvette->rotation = v4_quaternion_mul(corvette->rotation, v4_quaternion_yaw(sin(wave_time * 1.254) / 25));
  //corvette->rotation = v4_quaternion_mul(corvette->rotation, v4_quaternion_roll(sin(wave_time * 1.355) / 100));
  
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
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * 0.01;
    float a2 = (float)mouse_y * 0.01;
    
    cam->position = v3_sub(cam->position, cam->target);
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    cam->position = v3_add(cam->position, cam->target);
    
    cam->position = v3_sub(cam->position, cam->target);
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, v3_zero()) , v3(0,1,0) ));
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
    cam->position = v3_add(cam->position, cam->target);
    
  }
  
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
  static_object* s_corvette = entity_get("corvette");
  light* sun = entity_get("sun");
  light* backlight = entity_get("backlight");
  
  shadow_mapper_begin();
  shadow_mapper_render_static(s_corvette);
  shadow_mapper_end();
  
  forward_renderer_begin();
  
  forward_renderer_render_static(s_corvette);
  forward_renderer_render_static(s_seaplane);
  
  physics_object* balls[100];
  int num_balls;
  entities_get(balls, &num_balls, physics_object);
  for(int i = 0; i < num_balls; i++) {
    forward_renderer_render_physics(balls[i]);
  }
  
  static_object* center_sphere = entity_get("center_sphere");
  forward_renderer_render_static(center_sphere);
  
  forward_renderer_render_light(sun);
  forward_renderer_render_light(backlight);
  
  forward_renderer_end();
  
}

static int ball_count = 0;
void sea_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  case SDL_KEYUP:
  
    if (event.key.keysym.sym == SDLK_SPACE) {
      
      char ball_name[20];
      sprintf(ball_name, "ball_%i", ball_count);
      ball_count++;
      
      physics_object* ball = entity_new(ball_name, physics_object);
      ball->renderable = asset_get("./resources/ball.obj");
      ball->collision_body = collision_body_new_sphere(sphere_new(v3_zero(), 1));
      ball->position = cam->position;
      ball->scale = v3(0.5, 0.5, 0.5);
      ball->velocity = v3_mul(v3_normalize(v3_sub(cam->target, cam->position)), 75);
      
    }
  
  case SDL_MOUSEBUTTONDOWN:

    if (event.button.button == SDL_BUTTON_WHEELUP) {
      cam->position = v3_sub(cam->position, v3_normalize(cam->position));
    }
    if (event.button.button == SDL_BUTTON_WHEELDOWN) {
      cam->position = v3_add(cam->position, v3_normalize(cam->position));
    }
    
  break;
  
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
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
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
