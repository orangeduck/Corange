#include <stdio.h>

#include "corange.h"

static renderable* r_cello;
static renderable* r_piano;
static renderable* r_floor;
static renderable* r_skybox;

static font* console_font;
static ui_text* txt_framerate;
static ui_text* txt_renderer;
static ui_text* txt_info;

static camera* cam;
static light* sun;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static int use_piano = 1;

static int use_deferred = 1;

static void swap_renderer() {
  
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

void cello_init() {
  
  printf("Cello game init!\n");
  
  /* Get reference to the Cello */
  
  load_folder("/resources/cello/");
  load_folder("/resources/piano/");
  load_folder("/resources/floor/");
  load_folder("/resources/skybox/");
  load_folder("/resources/shaders/");
  
  model* cello = asset_get("/resources/cello/cello.obj");
  material* cello_mat = asset_get("/resources/cello/cello.mat");
  
  r_cello = renderable_new("cello");
  renderable_add_model(r_cello, cello);
  renderable_set_material(r_cello, cello_mat);
  
  model* piano = asset_get("/resources/piano/piano.obj");
  material* piano_mat = asset_get("/resources/piano/piano.mat");
  
  r_piano = renderable_new("piano");
  renderable_add_model(r_piano, piano);
  renderable_set_material(r_piano, piano_mat);
  
  model* floor = asset_get("/resources/floor/floor.obj");
  material* floor_mat = asset_get("/resources/floor/floor.mat");
  
  r_floor = renderable_new("floor");
  renderable_add_model(r_floor, floor);
  renderable_set_material(r_floor, floor_mat);
  
  model* skybox = asset_get("/resources/skybox/skybox.obj");
  material* skybox_mat = asset_get("/resources/skybox/skybox.mat");
  
  r_skybox = renderable_new("skybox");
  renderable_add_model(r_skybox, skybox);
  renderable_set_material(r_skybox, skybox_mat);
  
  /* Put some text on the screen */
  
  console_font = asset_get("./engine/fonts/console_font.fnt");
  
  txt_framerate = ui_text_new("hello", console_font);
  txt_framerate->position = v2(10, 10);
  ui_text_update(txt_framerate);
  
  txt_renderer = ui_text_new("Deferred Renderer", console_font);
  txt_renderer->position = v2(10, 30);
  ui_text_update(txt_renderer);
  
  txt_info = ui_text_new("Click and drag mouse to move\n'p' to switch object\n'r' to switch renderer", console_font);
  txt_info->position = v2(10, 50);
  ui_text_update(txt_info);
  
  /* Init render engine */
  
  viewport_set_vsync(1);
  //viewport_set_dimensions( v2(800 * 1.5, 600 * 1.5) );
  
  /* New Camera */
  
  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  
  sun = light_new_type( v3(30,43,-26), light_type_spot );
  
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  
  /* Renderer Setup */

  shadow_mapper_init(sun);  

  if (use_deferred) {
    deferred_renderer_init();
    deferred_renderer_set_camera(cam);
    deferred_renderer_set_light(sun);
    deferred_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  } else {
    forward_renderer_init();
    forward_renderer_set_camera(cam);
    forward_renderer_set_light(sun);
    forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  }
  
}

void cello_update() {

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
  
}

void cello_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_UP) { r_cello->position.y += 1; }
    if (event.key.keysym.sym == SDLK_DOWN) { r_cello->position.y -= 1; }
    if (event.key.keysym.sym == SDLK_p) { if(use_piano == 1){use_piano = 0;} else { use_piano = 1;} }
    if (event.key.keysym.sym == SDLK_r) { swap_renderer(); }
    
    if (event.key.keysym.sym == SDLK_LEFT) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(0.1) , r_cello->rotation); }
    if (event.key.keysym.sym == SDLK_RIGHT) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(-0.1) , r_cello->rotation); }   
        
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

void cello_render() {

  shadow_mapper_begin();
  if(use_piano) {
    shadow_mapper_render_renderable(r_piano);
    shadow_mapper_render_renderable(r_floor);
  } else {
    shadow_mapper_render_renderable(r_cello);
  }
  shadow_mapper_end();

  if (use_deferred) {
  
    deferred_renderer_begin();
    
    deferred_renderer_render_renderable(r_skybox);
    
    if(use_piano) {
      deferred_renderer_render_renderable(r_floor);
      deferred_renderer_render_renderable(r_piano);
    } else {
      deferred_renderer_render_renderable(r_cello);
    }
    deferred_renderer_end();
    
  } else {
  
    forward_renderer_begin();
    
    glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    forward_renderer_render_renderable(r_skybox);

    if(use_piano) {
      forward_renderer_render_renderable(r_floor);
      forward_renderer_render_renderable(r_piano);
    } else {
      forward_renderer_render_renderable(r_cello);
    }
    
    forward_renderer_end();
  }

  /* Render text */
    
  ui_text_render(txt_framerate);
  ui_text_render(txt_info);
  ui_text_render(txt_renderer);
  
}

void cello_finish() {

  renderable_delete(r_cello);
  renderable_delete(r_piano);
  renderable_delete(r_floor);
  renderable_delete(r_skybox);
  
  camera_delete(cam);
  light_delete(sun);

  ui_text_delete(txt_framerate);
  ui_text_delete(txt_info);
  ui_text_delete(txt_renderer);
  
  if (use_deferred) {
    deferred_renderer_finish();
  } else {
    forward_renderer_finish();
  }

  shadow_mapper_finish();

  printf("Cello game finish!\n");

}