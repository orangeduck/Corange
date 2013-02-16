#include <math.h>

#include "sea.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static deferred_renderer* g_dr;

void sea_init() {
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Sea");
  
  folder_load_recursive(P("./assets"));
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(5, 5, 5);
  cam->target =  vec3_new(0, 0, 0);
  
  asset_hndl opt_graphics = asset_hndl_new_load(P("./assets/graphics.cfg"));
  
  g_dr = deferred_renderer_new(opt_graphics);
  deferred_renderer_set_camera(g_dr, cam);
  deferred_renderer_set_tod(g_dr, 0.15);
  deferred_renderer_set_sea_enabled(g_dr, true);
  
  folder_load(P("./assets/corvette/"));
  
  renderable* r_corvette = asset_get(P("./assets/corvette/corvette.bmf"));
  r_corvette->material = asset_hndl_new_load(P("./assets/corvette/corvette.mat"));
  
  static_object* s_corvette = entity_new("corvette", static_object);
  s_corvette->renderable = asset_hndl_new_ptr(r_corvette);
  s_corvette->scale = vec3_new(1.0, 1.0, 1.0);
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "");
  ui_button_disable(framerate);
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  corvette->position.y = (sin(wave_time) + 1) / 2 - 1;
  corvette->rotation = mat4_rotation_x(sin(wave_time * 1.123) / 50);
  corvette->rotation = mat4_mul_mat4(corvette->rotation, mat4_rotation_y(sin(wave_time * 1.254) / 25));
  corvette->rotation = mat4_mul_mat4(corvette->rotation, mat4_rotation_z(sin(wave_time * 1.355) / 100));
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
}

void sea_render() {
  
  deferred_renderer_add(g_dr, render_object_static(entity_get("corvette")));
  deferred_renderer_render(g_dr);
  
}

static int ball_count = 0;
void sea_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  
  camera_control_orbit(cam, event);

}

void sea_finish() {
  deferred_renderer_delete(g_dr);
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  sea_init();
  
  int running = 1;
  SDL_Event e;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&e)) {
      switch(e.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (e.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (e.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        if (e.key.keysym.sym == SDLK_r &&
            e.key.keysym.mod == KMOD_LCTRL) {
            asset_reload_all();
        }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      sea_event(e);
      ui_event(e);
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
