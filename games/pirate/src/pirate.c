#include "corange.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

void pirate_init() {
  
  load_folder("/resources/");
  
  renderable* r_pirate = asset_get("/resources/pirate.smd");
  skeleton* skel_pirate = asset_get("/resources/pirate.skl");
  animation* ani_defend = asset_get("/resources/defend.ani");
  material* mat_pirate = asset_get("/resources/pirate.mat");
  
  renderable_set_material(r_pirate, mat_pirate);
  
  animated_object* pirate = entity_new("pirate", entity_type_animated);
  pirate->skeleton = skel_pirate;
  pirate->renderable = r_pirate;
  animated_object_add_animation(pirate, ani_defend);
  
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

}

void pirate_render() {

  animated_object* pirate = entity_get("pirate");

  shadow_mapper_begin();
  //shadow_mapper_render_animated(pirate);
  shadow_mapper_end();
  
  forward_renderer_begin();
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  forward_renderer_render_animated(pirate);
  
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