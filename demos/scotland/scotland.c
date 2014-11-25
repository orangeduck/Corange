
#include "corange.h"

static renderer* g_dr;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static bool toggle_freecam = true;
static bool loading_assets = false;
//static SDL_GLContext* load_context = NULL;

static int load_assets(void* unused) {
  
  //graphics_context_current(load_context);
  
  folder_load(P("./assets/terrain/"));
  folder_load(P("./assets/vegetation/"));
  
  landscape* world = entity_new("world", landscape);
  world->scale = 0.5;
  world->size_x = 512;
  world->size_y = 512;
  world->heightmap = asset_hndl_new_load(P("./assets/terrain/heightmap.raw"));
  world->attribmap = asset_hndl_new_load(P("./assets/terrain/attributemap.dds"));

  world->ground0 = asset_hndl_new_load(P("./assets/terrain/ground1.dds"));
  world->ground3 = asset_hndl_new_load(P("./assets/terrain/ground2.dds"));
  world->ground2 = asset_hndl_new_load(P("./assets/terrain/ground_earth.dds"));
  world->ground1 = asset_hndl_new_load(P("./assets/terrain/ground_path.dds"));
  
  world->ground0_nm = asset_hndl_new_load(P("./assets/terrain/ground1_nm.dds"));
  world->ground3_nm = asset_hndl_new_load(P("./assets/terrain/ground2_nm.dds"));
  world->ground2_nm = asset_hndl_new_load(P("./assets/terrain/ground_earth_nm.dds"));
  world->ground1_nm = asset_hndl_new_load(P("./assets/terrain/ground_path_nm.dds"));

  landscape_blobtree_generate(world);
  
  ui_button* loading = ui_elem_get("loading");
  ui_spinner* load_spinner = ui_elem_get("load_spinner");
  ui_button* framerate = ui_elem_get("framerate");
  ui_button* freecam = ui_elem_get("freecam");
  
  loading->active = false;
  load_spinner->active = false;
  framerate->active = true;
  freecam->active = true;
  
  loading_assets = false;
  //graphics_context_delete(load_context);
  return 1;
}

void scotland_init() {
  
  graphics_viewport_set_title("Scotland");
  graphics_viewport_set_size(1280, 720);
  
  ui_button* loading = ui_elem_new("loading", ui_button);
  ui_button_move(loading, vec2_new(graphics_viewport_width() / 2 - 40,graphics_viewport_height() / 2 - 13));
  ui_button_resize(loading, vec2_new(80,25));
  ui_button_set_label(loading, "Loading...");
  ui_button_disable(loading);
  
  ui_spinner* load_spinner = ui_elem_new("load_spinner", ui_spinner);
  load_spinner->color = vec4_white();
  load_spinner->top_left = vec2_new(graphics_viewport_width() / 2 + 50, graphics_viewport_height() / 2 - 13);
  load_spinner->bottom_right = vec2_add(load_spinner->top_left, vec2_new(24,24));
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  ui_button_set_active(framerate, false);
  
  ui_button* freecam = ui_elem_new("freecam", ui_button);
  ui_button_move(freecam, vec2_new(50,10));
  ui_button_resize(freecam, vec2_new(65,25));
  ui_button_set_label(freecam, "freecam");
  ui_button_set_active(freecam, false);
  
  void on_freecam(ui_button* b, void* unused) {
    toggle_freecam = !toggle_freecam;
    
    camera* cam = entity_get("camera");
    landscape* world = entity_get("world");
    
    vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
    float height = landscape_height(world, vec2_new(cam->position.x, cam->position.z));
    cam->position.y = height + 1;
    cam->target = vec3_add(cam->position, cam_dir);
  }
  
  ui_button_set_onclick(freecam, on_freecam);
  
  loading_assets = true;
  //load_context = graphics_context_new();
  //SDL_Thread* load_thread = SDL_CreateThread(load_assets, "loading", NULL);
  
  load_assets(NULL);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(0.0, 100.0, 0.0);
  cam->target =  vec3_new(50.0, 0.0,50.0);
  
  /* Renderer Setup */
  
  asset_hndl opt_graphics = asset_hndl_new_load(P("./assets/graphics.cfg"));
  
  g_dr = renderer_new(opt_graphics);
  renderer_set_camera(g_dr, cam);
  renderer_set_tod(g_dr, 0.15, 0);
  renderer_set_sea_enabled(g_dr, false);
  
}

static bool w_held = false;
static bool s_held = false;
static bool a_held = false;
static bool d_held = false;

void scotland_event(SDL_Event event) {
  
  camera* cam = entity_get("camera");

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
  landscape* world = entity_get("world");
  
  if (w_held || s_held) {
    
    vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
    float speed = 0.5;
    if (!toggle_freecam) speed = 0.05;
    if (w_held) {
      cam->position = vec3_add(cam->position, vec3_mul(cam_dir, speed));
    }
    if (s_held) {
      cam->position = vec3_sub(cam->position, vec3_mul(cam_dir, speed));
    }
    
    if (!toggle_freecam) {
      float height = landscape_height(world, vec2_new(cam->position.x, cam->position.z));
      cam->position.y = height + 1;
    }
    
    cam->target = vec3_add(cam->position, cam_dir);
  }
  
  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
    
    cam_dir.y += -a2;
    vec3 side_dir = vec3_normalize(vec3_cross(cam_dir, vec3_new(0,1,0)));
    cam_dir = vec3_add(cam_dir, vec3_mul(side_dir, -a1));
    cam_dir = vec3_normalize(cam_dir);
    
    cam->target = vec3_add(cam->position, cam_dir);
  }
  
  mouse_x = 0;
  mouse_y = 0;
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
}

void scotland_render() {
  
  camera* cam = entity_get("camera");
  landscape* world = entity_get("world");
  
  //texture_write_to_file(shadow_mapper_depth_texture(), "shadow_depth.tga");
  
  renderer_add(g_dr, render_object_landscape(world));
  renderer_render(g_dr);
  
}

void scotland_finish() {
  renderer_delete(g_dr);
}

int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  corange_init("../../assets_core");
  
  scotland_init();
  
  bool running = true;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = false; }
        if (event.key.keysym.sym == SDLK_PRINTSCREEN) { graphics_viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
      if (loading_assets) {
        ui_event(event);
      } else {
        scotland_event(event);
        ui_event(event);
      }
      
    }
    
    if (loading_assets) {
      ui_update();
    } else {
      scotland_update();
      ui_update();
    }
    
    
    if (loading_assets) {
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT);
      ui_render();
    } else {
      scotland_render();
      ui_render();
    }
    
    graphics_swap(); 
    
    frame_end();
  }
  
  scotland_finish();
  
  corange_finish();
  
  return 0;
}
