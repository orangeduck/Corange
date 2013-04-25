#define VOLUME_RENDERER

#include "particles.h"
#include "volume_renderer.h"
#include "marching_cubes.h"
#include "kernel.h"

#include "metaballs.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

void metaballs_init() {
  
  graphics_viewport_set_title("Metaballs");
  graphics_viewport_set_dimensions(1280, 720);
  graphics_set_multisamples(16);
  
#ifdef OPEN_GL_CPU
  kernels_init_with_cpu();
#else 
  kernels_init_with_opengl();
#endif
  
  asset_handler(kernel_program, "cl", cl_load_file, kernel_program_delete);
  
  folder_load(P("./kernels/"));
  
  metaball_particles_init();
  
  folder_load(P("./resources/podium/"));
  folder_load(P("./resources/particles/"));
  
  asset_hndl r_podium = asset_hndl_new(P("./resources/podium/podium.obj"));
  ((renderable*)asset_hndl_ptr(&r_podium))->material = asset_hndl_new(P("./resources/podium/podium.mat"));
  
  static_object* s_podium = entity_new("podium", static_object);
  s_podium->renderable = r_podium;
  s_podium->position = vec3_new(32, 10, 32);
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(50, 50, 50);
  cam->target = vec3_new(32, 15, 32);
  
  light* sun = entity_new("sun", light);
  sun->position = vec3_new(50,40,50);
  sun->ambient_color = vec3_new(0.5, 0.5, 0.5);
  sun->diffuse_color = vec3_mul(vec3_one(), 2);
  sun->specular_color = vec3_mul(vec3_one(), 5);
  light_set_type(sun, LIGHT_TYPE_SPOT);  
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "");
  ui_button_disable(framerate);
  
  ui_button* score = ui_elem_new("score", ui_button);
  ui_button_move(score, vec2_new(50, 10));
#ifdef VOLUME_RENDERER
  ui_button_resize(score, vec2_new(125, 25));
  ui_button_set_label(score, "Volume Renderer");
#endif
#ifdef MARCHING_CUBES
  ui_button_resize(score, vec2_new(120, 25));
  ui_button_set_label(score, "Marching Cubes");
#endif
#ifndef VOLUME_RENDERER
#ifndef MARCHING_CUBES
  ui_button_resize(score, vec2_new(80, 25));
  ui_button_set_label(score, "Particles");
#endif
#endif
  ui_button_disable(score);
  
#ifdef VOLUME_RENDERER
  volume_renderer_init();
  volume_renderer_set_camera(cam);
  volume_renderer_set_light(sun);
#endif
 
#ifdef MARCHING_CUBES
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_shadow_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  forward_renderer_add_light(sun);

  marching_cubes_init();
#endif
  
}

void metaballs_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(3)){
    sun->position.x += (float)mouse_y / 2;
    sun->position.z -= (float)mouse_x / 2;
  }

  mouse_x = 0;
  mouse_y = 0;

  metaball_particles_update(frame_time());
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
#ifdef MARCHING_CUBES
  marching_cubes_metaball_data( metaball_particle_positions_memory(), metaball_particles_count() );
  marching_cubes_clear();
  marching_cubes_update();
#endif

#ifdef VOLUME_RENDERER
  volume_renderer_metaball_data( metaball_particle_positions_memory(), metaball_particles_count() );
  volume_renderer_update();
#endif
  
}

static float proj_matrix[16];
static float view_matrix[16];

static bool wireframe = false;

void metaballs_render() {

  static_object* s_podium = entity_get("podium");
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
#ifdef MARCHING_CUBES 
  forward_renderer_begin();
    forward_renderer_render_static(s_podium);
    forward_renderer_render_light(sun);
    marching_cubes_render(wireframe, cam, sun);
  forward_renderer_end();
#endif
  
#ifdef VOLUME_RENDERER
  volume_renderer_render();
#endif
  
#ifndef VOLUME_RENDERER
#ifndef MARCHING_CUBES
  metaball_particles_render();
#endif
#endif
  
  ui_render();
  
  SDL_GL_SwapBuffers();
  
}

void metaballs_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  camera_control_orbit(cam, event);
  
  switch(event.type){
  
  case SDL_KEYDOWN:
    
  break;
  
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_SPACE) { 
      metaball_particles_reset();
    }
    
    if (event.key.keysym.sym == SDLK_w) {
      wireframe = !wireframe;
    }
    
  break;
  
  case SDL_MOUSEBUTTONUP:
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }

}

void metaballs_finish() {
  
  metaball_particles_finish();
  
#ifdef VOLUME_RENDERER
  volume_renderer_finish();
#endif
  
#ifdef MARCHING_CUBES
  forward_renderer_finish();
  marching_cubes_finish();
#endif
  
  kernels_finish();
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  metaballs_init();
  
  int running = 1;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { 
          #ifdef VOLUME_RENDERER
          volume_renderer_write_textures();
          #endif
          graphics_viewport_screenshot();
        }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      metaballs_event(event);
    }
    
    metaballs_update();
    
    metaballs_render();
    
    frame_end();
  }
  
  metaballs_finish();
  
  corange_finish();
  
  return 0;
}
