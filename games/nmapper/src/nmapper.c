#include "SDL/SDL.h"

#include "asset_manager.h"

#include "forward_renderer.h"

#include "camera.h"
#include "renderable.h"
#include "material.h"

static camera* cam;
static renderable* r_cello;
static material* nmapper_mat;
static float* strength;

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

void nmapper_init() {
  
  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  
  forward_renderer_init(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  forward_renderer_set_camera(cam);
  
  load_folder("/resources/");
  load_folder("/resources/meshes/");
  load_folder("/resources/textures/");
  
  nmapper_mat = asset_get("/resources/nmapper.mat");
  
  texture* t_cello = asset_get("/resources/textures/cello_nm.dds");
  model* m_cello = asset_get("/resources/meshes/cello.obj");
  
  r_cello = renderable_new("cello");
  renderable_add_model(r_cello, m_cello);
  
  strength = malloc(sizeof(float));
  *strength = 1.0f;
  
  renderable_set_material(r_cello, nmapper_mat);
  material_set_property(nmapper_mat, "bump_map", t_cello, mat_type_texture);
  material_set_property(nmapper_mat, "strength", strength, mat_type_float);
  
  glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
  
}

void nmapper_update() {

}

void nmapper_render() {

  forward_renderer_begin();
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  forward_renderer_render_renderable(r_cello);
  forward_renderer_end();

}

void nmapper_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
        
    if (event.key.keysym.sym == SDLK_UP) { cam->position.y += 1; }
    if (event.key.keysym.sym == SDLK_DOWN) { cam->position.y -= 1; }
    
    if (event.key.keysym.sym == SDLK_LEFT) { cam->position.z += 1; }
    if (event.key.keysym.sym == SDLK_RIGHT) { cam->position.z -= 1; }

    if (event.key.keysym.sym == SDLK_LEFTBRACKET) { cam->position.x += 1; }
    if (event.key.keysym.sym == SDLK_RIGHTBRACKET) { cam->position.x -= 1;}
    
    if (event.key.keysym.sym == SDLK_a) { r_cello->position.y += 1; }
    if (event.key.keysym.sym == SDLK_s) { r_cello->position.y -= 1; }
    
    if (event.key.keysym.sym == SDLK_q) { r_cello->scale = v3_add(r_cello->scale, v3(0.1, 0.1, 0.1)); }
    if (event.key.keysym.sym == SDLK_w) { r_cello->scale = v3_sub(r_cello->scale, v3(0.1, 0.1, 0.1)); }
    
    if (event.key.keysym.sym == SDLK_z) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(0.1) , r_cello->rotation); }
    if (event.key.keysym.sym == SDLK_x) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(-0.1) , r_cello->rotation); }        
        
    if (event.key.keysym.sym == SDLK_KP_PLUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength += 0.075f;
      *strength = *strength > 2.0f ? 2.0f : *strength;
    }
    if (event.key.keysym.sym == SDLK_KP_MINUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength -= 0.075f;
      *strength = *strength < 0.0f ? 0.0f : *strength;
    }
    
    break;
  }

}

void nmapper_finish() {

  free(strength);

  renderable_delete(r_cello);
  
  camera_delete(cam);
  
  forward_renderer_finish();

}