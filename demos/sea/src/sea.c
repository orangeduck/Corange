#include <math.h>

#include "sea.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static bool wireframe = false;

static bool wireframe_button_pressed = false;
static void switch_wireframe(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      wireframe_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (wireframe_button_pressed) {
      wireframe_button_pressed = false;
      rect->color = v4_black();
      
      if (wireframe == true) {
        wireframe = false;
      } else if (wireframe == false) {
        wireframe = true;
      }
      
    }
  }
}

void sea_init() {
  
  viewport_set_dimensions(1280, 720);

  camera* cam = entity_new("camera", camera);
  cam->position = v3(50.0, 50.0, 50.0);
  cam->target = v3(0, 5, 0);
  cam->near_clip = 0.1;
  
  light* sun = entity_new("sun", light);
  sun->position = v3(20,30,-20);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  light_set_type(sun, light_type_spot);
  
  shadow_mapper_init(sun);  
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  
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
  
  ui_rectangle* wireframe_rect = ui_elem_new("wireframe_rect", ui_rectangle);
  wireframe_rect->top_left = v2(10,10);
  wireframe_rect->bottom_right = v2(100, 35);
  wireframe_rect->color = v4_black();
  wireframe_rect->border_color = v4_white();
  wireframe_rect->border_size = 1;
  
  ui_elem_add_event("wireframe_rect", switch_wireframe);
  
  ui_text* wireframe_text = ui_elem_new("wireframe_text", ui_text);
  wireframe_text->position = v2(20, 15);
  wireframe_text->color = v4_white();
  ui_text_update_string(wireframe_text, "Wireframe");
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  corvette->position.y = (sin(wave_time) + 1) / 2;
  corvette->rotation = v4_quaternion_pitch(sin(wave_time * 1.123) / 50);
  corvette->rotation = v4_quaternion_mul(corvette->rotation, v4_quaternion_yaw(sin(wave_time * 1.254) / 25));
  corvette->rotation = v4_quaternion_mul(corvette->rotation, v4_quaternion_roll(sin(wave_time * 1.355) / 100));
  
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
  
}

void sea_render() {

  static_object* s_seaplane = entity_get("seaplane");
  static_object* s_corvette = entity_get("corvette");
  light* sun = entity_get("sun");
  
  shadow_mapper_begin();
  shadow_mapper_render_static(s_corvette);
  shadow_mapper_end();
  
  forward_renderer_begin();
  
  forward_renderer_render_static(s_corvette);
  
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }  
  forward_renderer_render_static(s_seaplane);
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  
  physics_object* balls[100];
  int num_balls;
  entities_get(balls, &num_balls, physics_object);
  for(int i = 0; i < num_balls; i++) {
    forward_renderer_render_physics(balls[i]);
  }
  
  static_object* center_sphere = entity_get("center_sphere");
  forward_renderer_render_static(center_sphere);
  
  forward_renderer_render_light(sun);
  
  forward_renderer_end();
  
}

static int ball_count = 0;
void sea_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_w) { if(wireframe == 1){wireframe = 0;} else { wireframe = 1;} }
  
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
    if (!wireframe_button_pressed) {
      mouse_x = event.motion.xrel;
      mouse_y = event.motion.yrel;
    }
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
