#include "corange.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static float animation_time = 0;

void pirate_init() {
  
  load_folder("/resources/pirate/");
  load_folder("/resources/floor/");
  load_folder("/resources/skybox/");
  
  renderable* r_pirate = asset_get("/resources/pirate/pirate.smd");
  material* mat_pirate = asset_get("/resources/pirate/pirate.mat");  
  renderable_set_material(r_pirate, mat_pirate);

  renderable* r_boots = asset_get("/resources/pirate/boots.smd");
  material* mat_boots = asset_get("/resources/pirate/boots.mat");  
  renderable_set_material(r_boots, mat_boots);
  
  animation* ani_cheer = asset_get("/resources/pirate/cheer.ani");
  skeleton* skel_pirate = asset_get("/resources/pirate/pirate.skl");
  
  animated_object* pirate = animated_object_new(r_pirate, skel_pirate);
  pirate->animation = ani_cheer;
  entity_add("pirate", entity_type_animated, pirate);
  
  animated_object* boots = animated_object_new(r_boots, skel_pirate);
  boots->animation = ani_cheer;
  entity_add("boots", entity_type_animated, boots);
  
  renderable* r_skybox = asset_get("/resources/skybox/skybox.obj");
  renderable_set_material(r_skybox, asset_get("/resources/skybox/skybox.mat"));
  entity_add("skybox", entity_type_static, static_object_new(r_skybox));
  
  renderable* r_floor = asset_get("/resources/floor/floor.obj");
  renderable_set_material(r_floor, asset_get("/resources/floor/floor.mat"));
  entity_add("floor", entity_type_static, static_object_new(r_floor));
  
  camera* cam = entity_new("camera", entity_type_camera);
  cam->position = v3(20.0, 20.0, 20.0);
  cam->target = v3(0.0, 10.0, 0.0);
  
  light* sun = entity_new("sun", entity_type_light);
  sun->position = v3(30, 43, 26);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  light_set_type(sun, light_type_spot);
  
  /* Renderer Setup */

  viewport_set_vsync(1);
  
  shadow_mapper_init(sun);
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  
}

void pirate_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * frame_time() * 0.25;
    float a2 = (float)mouse_y * frame_time() * 0.25;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  }
  
  if(keystate & SDL_BUTTON(3)){
    sun->position.x += (float)mouse_y / 2;
    sun->position.z -= (float)mouse_x / 2;
  }

  mouse_x = 0;
  mouse_y = 0;

  animated_object* pirate = entity_get("pirate");
  animated_object* boots = entity_get("boots");
  animated_object_update(pirate, 0.5);
  animated_object_update(boots, 0.5);
}

void pirate_render() {

  animated_object* pirate = entity_get("pirate");
  animated_object* boots = entity_get("boots");
  static_object* skybox = entity_get("skybox");
  static_object* floor = entity_get("floor");
  
  shadow_mapper_begin();
  shadow_mapper_render_animated(pirate);
  shadow_mapper_render_animated(boots);
  shadow_mapper_end();
  
  forward_renderer_begin();
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  forward_renderer_render_static(skybox);
  forward_renderer_render_static(floor);
  forward_renderer_render_animated(pirate);
  forward_renderer_render_animated(boots);
  forward_renderer_render_skeleton(pirate->pose);
  forward_renderer_end();

}

void pirate_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  switch(event.type){
  case SDL_KEYUP:
    
  break;

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

void pirate_finish() {

  forward_renderer_finish();
  shadow_mapper_finish();

}