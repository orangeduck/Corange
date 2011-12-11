#include "corange.h"

static ui_text* txt_framerate;
static ui_text* txt_renderer;
static ui_text* txt_info;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static int use_deferred = 1;

static void swap_renderer() {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  if (use_deferred) {
    
    deferred_renderer_finish();
    forward_renderer_init();
    forward_renderer_set_camera(cam);
    forward_renderer_set_light(sun);
    forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    
    ui_text_update_string(txt_renderer,"Forward Renderer");
    
    use_deferred = 0;
    
  } else {
    
    forward_renderer_finish();
    deferred_renderer_init();
    deferred_renderer_set_camera(cam);
    deferred_renderer_set_light(sun);
    deferred_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    
    ui_text_update_string(txt_renderer,"Deferred Renderer");
    
    use_deferred = 1;
  }

}

void pirate_init() {
  
  load_folder("/resources/imrod/");
  load_folder("/resources/pirate/");
  load_folder("/resources/floor/");
  load_folder("/resources/skybox/");
  
  renderable* r_imrod = asset_get("/resources/imrod/imrod.smd");
  material* mat_imrod = asset_get("/resources/imrod/imrod_animated.mat");  
  renderable_set_material(r_imrod, mat_imrod);
  
  skeleton* skel_imrod = asset_get("/resources/imrod/imrod.skl");
  animation* ani_stand = asset_get("/resources/pirate/stand.ani");
  
  animated_object* imrod = animated_object_new(r_imrod, skel_imrod);
  imrod->animation = ani_stand;
  entity_add("imrod", entity_type_animated, imrod);
  
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
  
  /* Text */
  
  font* console_font = asset_get("./engine/fonts/console_font.fnt");
  
  txt_framerate = ui_text_new("framerate", console_font);
  txt_framerate->position = v2(10, 10);
  ui_text_update(txt_framerate);
  
  txt_renderer = ui_text_new("Deferred Renderer", console_font);
  txt_renderer->position = v2(10, 30);
  ui_text_update(txt_renderer);
  
  txt_info = ui_text_new("Click and drag mouse to move\n'p' to switch object\n'r' to switch renderer", console_font);
  txt_info->position = v2(10, 50);
  ui_text_update(txt_info);
  
  /* Renderer Setup */

  viewport_set_vsync(1);
  viewport_set_multisamples(16);
  
  shadow_mapper_init(sun);
  
  deferred_renderer_init();
  deferred_renderer_set_camera(cam);
  deferred_renderer_set_light(sun);
  deferred_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  
  use_deferred = 1;
  
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

  ui_text_update_string(txt_framerate, frame_rate_string());
  
  animated_object* imrod = entity_get("imrod");
  animated_object_update(imrod, 0.5);
}

void pirate_render() {


  static_object* skybox = entity_get("skybox");
  static_object* floor = entity_get("floor");
  animated_object* imrod = entity_get("imrod");
  
  shadow_mapper_begin();
  shadow_mapper_render_animated(imrod);
  shadow_mapper_render_static(floor);
  shadow_mapper_end();

  if (use_deferred) {
  
    deferred_renderer_begin();
    deferred_renderer_render_static(skybox);
    deferred_renderer_render_static(floor);
    deferred_renderer_render_animated(imrod);
    deferred_renderer_end();
    
  } else {
  
    forward_renderer_begin();
    
    forward_renderer_render_static(skybox);
    forward_renderer_render_static(floor);
    forward_renderer_render_animated(imrod);
    forward_renderer_end();
  }

  ui_text_render(txt_framerate);
  ui_text_render(txt_info);
  ui_text_render(txt_renderer);
  
}

void pirate_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  switch(event.type){
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_r) { swap_renderer(); }
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
  
  ui_text_delete(txt_framerate);
  ui_text_delete(txt_info);
  ui_text_delete(txt_renderer);
  
  if (use_deferred) {
    deferred_renderer_finish();
  } else {
    forward_renderer_finish();
  }

  shadow_mapper_finish();

}