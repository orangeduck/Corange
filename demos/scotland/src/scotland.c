#include <stdio.h>

#include "scotland.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static bool wireframe = false;
static bool freecam = true;

static void toggle_wireframe(ui_button* b, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_button_contains_position(b, v2(event.motion.x, event.motion.y))) {
      b->pressed = true;
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (b->pressed) {
      b->pressed = false;
      
      wireframe = !wireframe;
    }
  }
}

static void toggle_freecam(ui_button* b, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_button_contains_position(b, v2(event.motion.x, event.motion.y))) {
      b->pressed = true;
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (b->pressed) {
      b->pressed = false;
      
      freecam = !freecam;
      
      camera* cam = entity_get("camera");
      landscape* world = entity_get("world");
      
      vector3 cam_dir = v3_normalize(v3_sub(cam->target, cam->position));
      float height = terrain_height(world->terrain, v2(cam->position.x, cam->position.z));
      cam->position.y = height + 1;
      cam->target = v3_add(cam->position, cam_dir);
    }
  }
}

void scotland_init() {
  
  viewport_set_dimensions(1280, 720);
  
  load_folder("./resources/terrain/");
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, v2(10,10));
  ui_button_resize(framerate, v2(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  
  ui_button* wireframe = ui_elem_new("wireframe", ui_button);
  ui_button_move(wireframe, v2(50,10));
  ui_button_resize(wireframe, v2(80,25));
  ui_button_set_label(wireframe, "wireframe");
  
  ui_elem_add_event("wireframe", toggle_wireframe);
  
  ui_button* freecam = ui_elem_new("freecam", ui_button);
  ui_button_move(freecam, v2(140,10));
  ui_button_resize(freecam, v2(65,25));
  ui_button_set_label(freecam, "freecam");
  
  ui_elem_add_event("freecam", toggle_freecam);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(512.0, 200.0, 512.0);
  cam->target =  v3(0.0, 0.0, 0.0);
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_sun);
  
  static_object* skydome = entity_new("skydome", static_object);
  skydome->renderable = asset_get("./resources/terrain/skydome.obj");
  renderable_set_material(skydome->renderable, asset_get("./resources/terrain/skydome.mat"));
  
  landscape* world = entity_new("world", landscape);
  world->terrain = asset_get("./resources/terrain/heightmap.raw");
  world->normalmap = asset_get("./resources/terrain/normalsmap.dds");
  world->colormap = asset_get("./resources/terrain/colormap.dds");
  landscape_add_surface(world, asset_get("./resources/terrain/texture.dds"), 
                               asset_get("./resources/terrain/texture_nm.dds"), 
                               asset_get("./resources/terrain/texture_far.dds"), 
                               asset_get("./resources/terrain/texture_far_nm.dds"));
  
  float height = terrain_height(world->terrain, v2(cam->position.x, cam->position.z));
  cam->position.y = height + 1;
  
  /* Renderer Setup */
  
  shadow_mapper_init(sun);
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_shadow_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  forward_renderer_add_light(sun);
  
}

static bool w_held = false;
static bool s_held = false;
static bool a_held = false;
static bool d_held = false;

void scotland_event(SDL_Event event) {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_w) { w_held = true; }
    if (event.key.keysym.sym == SDLK_a) { a_held = true; }
    if (event.key.keysym.sym == SDLK_s) { s_held = true; }
    if (event.key.keysym.sym == SDLK_d) { d_held = true; }
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_w) { w_held = false; }
    if (event.key.keysym.sym == SDLK_a) { a_held = false; }
    if (event.key.keysym.sym == SDLK_s) { s_held = false; }
    if (event.key.keysym.sym == SDLK_d) { d_held = false; }
  break;

  case SDL_MOUSEBUTTONDOWN:
  break;
  
  case SDL_MOUSEBUTTONUP:
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }
    
}

void scotland_update() {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  static_object* skydome = entity_get("skydome");
  landscape* world = entity_get("world");
  
  skydome->position.x = cam->position.x;
  skydome->position.y = 0;
  skydome->position.z = cam->position.z;
  
  sun->position.x = 512;
  sun->position.y = 1024;
  sun->position.z = 512;
  
  sun->target = v3_add(sun->position, v3(1,-2,1));
  
  if (w_held || s_held) {
    
    vector3 cam_dir = v3_normalize(v3_sub(cam->target, cam->position));
    float speed = 0.5;
    if (!freecam) speed = 0.1;
    if (w_held) {
      cam->position = v3_add(cam->position, v3_mul(cam_dir, speed));
    }
    if (s_held) {
      cam->position = v3_sub(cam->position, v3_mul(cam_dir, speed));
    }
    
    if (!freecam) {
      float height = terrain_height(world->terrain, v2(cam->position.x, cam->position.z));
      cam->position.y = height + 1;
    }
    
    cam->target = v3_add(cam->position, cam_dir);
  }
  
  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    vector3 cam_dir = v3_normalize(v3_sub(cam->target, cam->position));
    
    cam_dir.y += -a2;
    vector3 side_dir = v3_normalize(v3_cross(cam_dir, v3(0,1,0)));
    cam_dir = v3_add(cam_dir, v3_mul(side_dir, -a1));
    cam_dir = v3_normalize(cam_dir);
    
    cam->target = v3_add(cam->position, cam_dir);
  }
  
  mouse_x = 0;
  mouse_y = 0;
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
}

void scotland_render() {
  
  light* sun = entity_get("sun");
  camera* cam = entity_get("camera");
  landscape* world = entity_get("world");
  static_object* skydome = entity_get("skydome");
  
  shadow_mapper_begin();
  shadow_mapper_render_landscape(world);
  shadow_mapper_end();

  forward_renderer_begin();
  forward_renderer_render_static(skydome);
  forward_renderer_render_light(sun);
  
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  forward_renderer_render_landscape(world);
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  
  forward_renderer_end();
  
}

void scotland_finish() {

  forward_renderer_finish();
  shadow_mapper_finish();
  
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  scotland_init();
  
  int running = 1;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      scotland_event(event);
      ui_event(event);
    }
    
    scotland_update();
    ui_update();
    
    scotland_render();
    ui_render();
    
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }
  
  scotland_finish();
  
  corange_finish();
  
  return 0;
}
