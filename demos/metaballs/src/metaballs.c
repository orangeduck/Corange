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
  
  asset_manager_handler(kernel_program, "cl", cl_load_file, kernel_program_delete);
  
  load_folder("./kernels/");
  
  particles_init();
  
  load_folder("./resources/podium/");
  load_folder("./resources/particles/");
  
  renderable* r_podium = asset_get("./resources/podium/podium.obj");
  renderable_set_material(r_podium, asset_get("./resources/podium/podium.mat"));
  
  static_object* s_podium = entity_new("podium", static_object);
  s_podium->renderable = r_podium;
  s_podium->position = v3(32, 10, 32);
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(50, 50, 50);
  cam->target = v3(32, 15, 32);
  
  light* sun = entity_new("sun", light);
  sun->position = v3(50,40,50);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3_mul(v3_one(), 2);
  sun->specular_color = v3_mul(v3_one(), 5);
  light_set_type(sun, light_type_spot);  
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, v2(10,10));
  ui_button_resize(framerate, v2(30,25));
  ui_button_set_label(framerate, "");
  ui_button_disable(framerate);
  
  ui_button* score = ui_elem_new("score", ui_button);
  ui_button_move(score, v2(50, 10));
#ifdef VOLUME_RENDERER
  ui_button_resize(score, v2(125, 25));
  ui_button_set_label(score, "Volume Renderer");
#endif
#ifdef MARCHING_CUBES
  ui_button_resize(score, v2(120, 25));
  ui_button_set_label(score, "Marching Cubes");
#endif
#ifndef VOLUME_RENDERER
#ifndef MARCHING_CUBES
  ui_button_resize(score, v2(80, 25));
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
  shadow_mapper_init(sun);  
  
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
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * frame_time() * 0.25;
    float a2 = (float)mouse_y * frame_time() * 0.25;
    
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

  particles_update(frame_time());
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
#ifdef MARCHING_CUBES
  marching_cubes_metaball_data( particle_positions_memory(), particles_count() );
  marching_cubes_clear();
  marching_cubes_update();
#endif

#ifdef VOLUME_RENDERER
  volume_renderer_metaball_data( particle_positions_memory(), particles_count() );
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
  shadow_mapper_begin();
    shadow_mapper_render_static(s_podium);
    marching_cubes_render_shadows(sun);
  shadow_mapper_end();
  
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
  particles_render();
#endif
#endif
  
  ui_render();
  
  SDL_GL_SwapBuffers();
  
}

void metaballs_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  
  case SDL_KEYDOWN:
    
  break;
  
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_SPACE) { 
      particles_reset();
    }
    
    if (event.key.keysym.sym == SDLK_w) {
      wireframe = !wireframe;
    }
    
  break;
  
  case SDL_MOUSEBUTTONUP:
    
  break;
  
  case SDL_MOUSEBUTTONDOWN:

    if (event.button.button == SDL_BUTTON_WHEELUP) {
      cam->position = v3_sub(cam->position, v3_normalize(v3_sub(cam->position, cam->target)));
    }
    if (event.button.button == SDL_BUTTON_WHEELDOWN) {
      cam->position = v3_add(cam->position, v3_normalize(v3_sub(cam->position, cam->target)));
    }
    
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }

}

void metaballs_finish() {
  
  particles_finish();
  
#ifdef VOLUME_RENDERER
  volume_renderer_finish();
#endif
  
#ifdef MARCHING_CUBES
  shadow_mapper_finish();
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
