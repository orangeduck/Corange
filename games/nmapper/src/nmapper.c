#include "SDL/SDL.h"

#include "asset_manager.h"

#include "forward_renderer.h"

#include "light.h"
#include "camera.h"
#include "matrix.h"
#include "renderable.h"
#include "material.h"
#include "timing.h"
#include "text_renderer.h"

#include "viewport.h"

static camera* cam;
static light* sun;

static material* nmapper_mat;
static float* strength;

static char* strength_string;
static char* fov_string;

static render_text* rt_strength;
static render_text* rt_fov;

static renderable* r_cello;
static renderable* r_torus;

void nmapper_init() {
  
  viewport_set_multisamples(8);
  viewport_set_width(512);
  viewport_set_height(512);
  
  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  sun = light_new_type( v3(30,43,-26), light_type_spot );
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  
  load_folder("/resources/");
  load_folder("/resources/meshes/");
  load_folder("/resources/textures/");
  
  nmapper_mat = asset_get("/resources/nmapper.mat");
  
  texture* t_cello = asset_get("/resources/textures/cello_nm.dds");
  texture* t_blank = asset_get("/resources/textures/blank_nm.dds");
  
  model* m_cello = asset_get("/resources/meshes/cello.obj");
  model* m_torus = asset_get("/resources/meshes/torus.obj");
  
  printf("TEST1");fflush(stdout);
 
  r_cello = renderable_new("cello");
  renderable_add_model(r_cello, m_cello);
  renderable_set_material(r_cello, nmapper_mat);

  printf("TEST2");fflush(stdout);
  
  r_torus = renderable_new("torus");
  renderable_add_model(r_torus, m_torus);
  renderable_set_material(r_torus, nmapper_mat);
  
  printf("TEST3");fflush(stdout);
  
  strength = malloc(sizeof(float));
  *strength = 1.0f;
  
  strength_string = malloc(128);
  fov_string = malloc(128);
  strcpy(strength_string,"");
  strcpy(fov_string,"");

  printf("TEST4");fflush(stdout);
  
  font* console_font = asset_get("./engine/fonts/console_font.fnt");
  
  rt_strength = render_text_new(strength_string, 128, console_font);
  rt_strength->position = v2(-1.0,-1.0);
  rt_strength->scale = v2(1.0,1.0);
  rt_strength->color = v4(1,1,1,1);
  render_text_update(rt_strength);
  
  rt_fov = render_text_new(fov_string, 128, console_font);
  rt_fov->position = v2(-1.0,-0.93);
  rt_fov->scale = v2(1.0,1.0);
  rt_fov->color = v4(1,1,1,1);
  render_text_update(rt_fov);
  
  printf("TEST5");fflush(stdout);
  
  material_set_property(nmapper_mat, "bump_map", t_cello, mat_type_texture);
  //material_set_property(nmapper_mat, "bump_map", t_blank, mat_type_texture);
  material_set_property(nmapper_mat, "strength", strength, mat_type_float);
  
  glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
    
  printf("TEST6");fflush(stdout);

}

static int mouse_down;
static int mouse_x;
static int mouse_y;

void nmapper_update() {

  #define PI 3.14159265

  sprintf(strength_string, "Strength: %.2f", *strength);
  sprintf(fov_string, "FOV: %.2f degrees", cam->fov * (180.0 / PI) );

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  } 

  mouse_x = 0;
  mouse_y = 0;
  
}

void nmapper_render() {

  forward_renderer_begin();
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  forward_renderer_render_renderable(r_cello);
  //forward_renderer_render_renderable(r_torus);
  forward_renderer_end();
  
  render_text_update_string(rt_strength, strength_string);
  render_text_update_string(rt_fov, fov_string);
  
  render_text_render(rt_strength);
  render_text_render(rt_fov);

}

void nmapper_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_q) {
      cam->fov -= 0.01;
      cam->fov = cam->fov < 0.78 ? 0.78 : cam->fov;
      return;
    }
    
    if (event.key.keysym.sym == SDLK_w) {
      cam->fov += 0.01;
      cam->fov = cam->fov > 1.22 ? 1.22 : cam->fov;
      return;
    }
        
    if (event.key.keysym.sym == SDLK_KP_PLUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength += 0.075f;
      *strength = *strength > 2.0f ? 2.0f : *strength;
      return;
    }
    if (event.key.keysym.sym == SDLK_KP_MINUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength -= 0.075f;
      *strength = *strength < 0.0f ? 0.0f : *strength;
      return;
    }
    break;
  
  case SDL_MOUSEBUTTONDOWN:
  
    if (event.button.button == SDL_BUTTON_LEFT) {
      mouse_down = 1;
      
    } else if (event.button.button == SDL_BUTTON_WHEELUP) {
      
      cam->position = v3_sub(cam->position, v3_normalize(cam->position));
      
    } else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
    
      cam->position = v3_add(cam->position, v3_normalize(cam->position));
      
    } 
    
  break;
  
  case SDL_MOUSEBUTTONUP:
    if (event.button.button == SDL_BUTTON_LEFT) {
      mouse_down = 0;
    }
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  
  }
  
}

void nmapper_finish() {

  free(strength);

  render_text_delete(rt_strength);
  render_text_delete(rt_fov);
  
  renderable_delete(r_cello);
  renderable_delete(r_torus);
  
  camera_delete(cam);
  
  forward_renderer_finish();

}