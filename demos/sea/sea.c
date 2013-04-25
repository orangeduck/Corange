#include "corange.h"

#include <math.h>

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static deferred_renderer* g_dr;

static ellipsoid test_ellipsoid;
static vec3 test_velocity;
static cmesh* test_cmesh;
static mat4 test_cmesh_trans;

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
  deferred_renderer_set_tod(g_dr, 0.15, 0);
  deferred_renderer_set_sea_enabled(g_dr, true);
  
  folder_load(P("./assets/corvette/"));
  
  renderable* r_corvette = asset_get(P("./assets/corvette/corvette.bmf"));
  r_corvette->material = asset_hndl_new_load(P("./assets/corvette/corvette.mat"));
  
  static_object* s_corvette = entity_new("corvette", static_object);
  s_corvette->renderable = asset_hndl_new_ptr(r_corvette);
  s_corvette->scale = vec3_new(2.0, 2.0, 2.0);
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "");
  ui_button_disable(framerate);
  
  test_ellipsoid = ellipsoid_new(vec3_new(0, 10, 5), vec3_new(1,2,1));
  test_velocity = vec3_zero();
  test_cmesh = asset_get(P("./assets/corvette/corvette.col"));
  test_cmesh_trans = mat4_scale(vec3_new(2, 2, 2));
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  //corvette->position.y = (sin(wave_time) + 1) / 2 - 1;
  //corvette->rotation = quat_rotation_x(sin(wave_time * 1.123) / 50);
  //corvette->rotation = quat_mul_quat(corvette->rotation, quat_rotation_y(sin(wave_time * 1.254) / 25));
  //corvette->rotation = quat_mul_quat(corvette->rotation, quat_rotation_z(sin(wave_time * 1.355) / 100));
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
  Uint8* keystate = SDL_GetKeyState(NULL);
  
  vec3 right  = vec3_normalize(vec3_cross(camera_direction(cam), vec3_up()));
  vec3 left = vec3_neg(right);
  vec3 front = vec3_cross(left, vec3_up());
  vec3 back  = vec3_neg(front);
  vec3 movement = vec3_zero();
  
  if (keystate[SDLK_w]) {
    movement = vec3_add(movement, front);
  }
  if (keystate[SDLK_a]) {
    movement = vec3_add(movement, left);
  }
  if (keystate[SDLK_s]) {
    movement = vec3_add(movement, back);
  }
  if (keystate[SDLK_d]) {
    movement = vec3_add(movement, right);
  }
  
  /*
  if (keystate[SDLK_UP]) {
    test_velocity = vec3_add(test_velocity, vec3_mul(vec3_up(), frame_time()));
  }
  if (keystate[SDLK_DOWN]) {
    test_velocity = vec3_add(test_velocity, vec3_mul(vec3_neg(vec3_up()), frame_time()));
  }
  */
  
  float top_speed = 0.25;
  
  vec3 gravity_update  = vec3_add(test_velocity, vec3_mul(vec3_gravity(), frame_time()));
  vec3 movement_update = vec3_lerp(test_velocity, vec3_mul(movement, top_speed), saturate(5 * frame_time()));
  
  collision col = ellipsoid_collide_mesh(test_ellipsoid, vec3_new(0, -1, 0), test_cmesh, test_cmesh_trans);
  float closeness = col.collided ? clamp((1 - col.time), 0.01, 0.99) : 0.01;
  
  test_velocity = vec3_lerp(gravity_update, movement_update, closeness);
  
  /* Collision Detection and response routine */
  
  deferred_renderer_add(g_dr, render_object_cmesh(test_cmesh, test_cmesh_trans));
  deferred_renderer_add(g_dr, render_object_ellipsoid(test_ellipsoid));
  
  col = ellipsoid_collide_mesh(test_ellipsoid, test_velocity, test_cmesh, test_cmesh_trans);
  
  int count = 0;
  while (col.collided) {
    
    deferred_renderer_add(g_dr, render_object_point(test_ellipsoid.center, vec3_white(), 5));
    deferred_renderer_add(g_dr, render_object_point(col.point, vec3_red(), 10));
    
    if (count++ == 5) {
      warning("Panic out of collision detection!");
      test_velocity = vec3_zero();
      break;
    }
    
    if (vec3_length(test_velocity) < 0.001) {
      test_velocity = vec3_zero();
      break;
    }
    
    vec3 fwrd = vec3_mul(test_velocity, col.time);
    vec3 dest = vec3_add(test_ellipsoid.center, fwrd);
    
    deferred_renderer_add(g_dr, render_object_line(col.point, vec3_add(col.point, col.norm), vec3_green(), 3));
    
    float len = max(vec3_length(fwrd) - 0.01, 0.0);
    vec3 move = vec3_add(test_ellipsoid.center, vec3_mul(vec3_normalize(fwrd), len));    
    vec3 proj = vec3_project(vec3_mul(test_velocity, (1-col.time)), col.norm);
    
    deferred_renderer_add(g_dr, render_object_line(test_ellipsoid.center, vec3_add(test_ellipsoid.center, vec3_normalize(proj)), vec3_blue(), 3));
    
    test_ellipsoid.center = move;
    test_velocity = proj;
    
    // Re-detect
    
    col = ellipsoid_collide_mesh(test_ellipsoid, test_velocity, test_cmesh, test_cmesh_trans);
  
  }
  
  test_ellipsoid.center = vec3_add(test_ellipsoid.center, test_velocity);
  
  /* End */
  
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
  
  corange_init("../../assets_core");
  
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
