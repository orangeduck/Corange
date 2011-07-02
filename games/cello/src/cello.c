#include <stdio.h>

#include "SDL/SDL.h"

#include "asset_manager.h"
#include "geometry.h"
#include "material.h"

#include "text_renderer.h"
#include "forward_renderer.h"
#include "deferred_renderer.h"
#include "painting_renderer.h"

#include "font.h"
#include "timing.h"
#include "scripting.h"

#include "painting_renderable.h"
#include "renderable.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

static model* cello;
static model* piano;

static render_model* rm_cello;
static render_model* rm_piano;

static material* cello_mat;
static material* piano_mat;

static renderable* r_cello;
static renderable* r_piano;

static painting_renderable* pr_cello;
static painting_renderable* pr_piano;

static font* console_font;
static render_text* rt_framerate;
static render_text* rt_test_text;

static camera* cam;

static int mouse_x;
static int mouse_y;
static int mouse_down;

static int use_piano = 0;

void cello_init() {
  
  printf("Cello game init!\n");
  
  /* New Camera */
  
  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  
  /* Renderer Setup */

#define PAINTING_RENDER

#ifdef DEFERRED_RENDER
  deferred_renderer_init();
  deferred_renderer_set_camera(cam);
#endif

#ifdef FORWARD_RENDER  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
#endif

#ifdef PAINTING_RENDER
  painting_renderer_init();
  painting_renderer_set_camera(cam);
#endif
  
  /* Script stuff */
  
  script* s = asset_get("./engine/scripts/hello_world.lua");
  scripting_run_script(s);
  
  /* Get reference to the Cello */
  
  load_folder("/resources/cello/");
  load_folder("/resources/piano/");
  load_folder("/resources/shaders/");
  
  cello = asset_get("/resources/cello/cello.obj");
  cello_mat = asset_get("/resources/cello/cello.mat");
  
  piano = asset_get("/resources/piano/piano.obj");
  piano_mat = asset_get("/resources/piano/piano.mat");
  
  rm_cello = to_render_model(cello);
  rm_piano = to_render_model(piano);
    
  r_cello = renderable_new("cello");
  renderable_add_model(r_cello, cello);
  renderable_set_material(r_cello, cello_mat);
  
  pr_cello = painting_renderable_new("paint_cello");
  painting_renderable_add_model(pr_cello, cello);
  renderable_set_material(pr_cello->renderable, cello_mat);
  
  r_piano = renderable_new("piano");
  renderable_add_model(r_piano, piano);
  renderable_set_material(r_piano, piano_mat);
  
  pr_piano = painting_renderable_new("paint_piano");
  painting_renderable_add_model(pr_piano, piano);
  renderable_set_material(pr_piano->renderable, piano_mat);
  
  /* Put some text on the screen */
  
  console_font = asset_get("./engine/fonts/console_font.fnt");
  
  rt_framerate = render_text_new("hello", 10, console_font);
  rt_framerate->position = v2(-1.0,-1.0);
  rt_framerate->scale = v2(1.0,1.0);
  rt_framerate->color = v4(0,0,0,1);
  render_text_update(rt_framerate);
  
  rt_test_text = render_text_new("Corange v0.1", 512, console_font);
  rt_test_text->position = v2(-1.0,-0.95);
  rt_test_text->color = v4(0,0,1,1);
  render_text_update(rt_test_text);
  
  
}

void cello_update() {

  #define PI 3.14159265

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * frame_time() * 2;
    float a2 = (float)mouse_y * frame_time() * 2;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  } 

  mouse_x = 0;
  mouse_y = 0;
  
}

void cello_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_UP) { r_cello->position.y += 1; }
    if (event.key.keysym.sym == SDLK_DOWN) { r_cello->position.y -= 1; }
    if (event.key.keysym.sym == SDLK_p) { if(use_piano == 1){use_piano = 0;} else { use_piano = 1;} }
    
    if (event.key.keysym.sym == SDLK_LEFT) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(0.1) , r_cello->rotation); }
    if (event.key.keysym.sym == SDLK_RIGHT) { r_cello->rotation = v4_quaternion_mul(
        v4_quaternion_yaw(-0.1) , r_cello->rotation); }   
        
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

void cello_render() {

#ifdef DEFERRED_RENDER
  deferred_renderer_begin();
  if(use_piano) {
    deferred_renderer_render_renderable(r_piano);
  } else {
    deferred_renderer_render_renderable(r_cello);
  }
  deferred_renderer_end();
#endif
  
#ifdef FORWARD_RENDER
  forward_renderer_begin();
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  if(use_piano) {
    forward_renderer_render_renderable(r_piano);
  } else {
    forward_renderer_render_renderable(r_cello);
  }
  
  forward_renderer_end();
#endif

#ifdef PAINTING_RENDER

  if(use_piano) {
  
    painting_renderer_begin_render();
    painting_renderer_render_renderable(pr_piano);
    painting_renderer_end_render();

    painting_renderer_begin_painting();
    painting_renderer_paint_renderable(pr_piano);
    painting_renderer_end_painting();
    

  } else {

    painting_renderer_begin_render();
    painting_renderer_render_renderable(pr_cello);
    painting_renderer_end_render();

    painting_renderer_begin_painting();
    painting_renderer_paint_renderable(pr_cello);
    painting_renderer_end_painting();
  
  }

#endif

  /* Render text */
  
  render_text_update_string(rt_framerate, frame_rate_string());
  
  render_text_render(rt_framerate);
  render_text_render(rt_test_text);
  
}

void cello_finish() {

  render_model_delete(rm_cello);
  renderable_delete(r_cello);
  painting_renderable_delete(pr_cello);
  
  render_model_delete(rm_piano);
  renderable_delete(r_piano);
  painting_renderable_delete(pr_piano);
  
  camera_delete(cam);

  render_text_delete(rt_framerate);
  render_text_delete(rt_test_text);
  
#ifdef DEFERRED_RENDER
  deferred_renderer_finish();
#endif
    
#ifdef FORWARD_RENDER
  forward_renderer_finish();
#endif

#ifdef PAINTING_RENDER
  painting_renderer_finish();
#endif

  printf("Cello game finish!\n");

}