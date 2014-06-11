
#include "corange.h"

static int object_id = 0;
deferred_renderer* dr = NULL;

void renderers_init() {
  
  graphics_viewport_set_title("Renderers");
  graphics_viewport_set_size(1280, 720);
  
  folder_load(P("./assets/podium/"));
  folder_load(P("./assets/cello/"));
  folder_load(P("./assets/piano/"));
  folder_load(P("./assets/imrod/"));
  folder_load(P("./assets/dino/"));
  
  static_object* s_podium = entity_new("podium", static_object);
  s_podium->renderable = asset_hndl_new(P("./assets/podium/podium.bmf"));
  
  static_object* s_cello = entity_new("cello", static_object);
  s_cello->renderable = asset_hndl_new(P("./assets/cello/cello.bmf"));
  s_cello->position = vec3_new(0, 3, 0);
  s_cello->rotation = quat_rotation_x(-1.7);
  s_cello->scale = vec3_new(0.75, 0.75, 0.75);

  static_object* s_piano = entity_new("piano", static_object);
  s_piano->renderable = asset_hndl_new(P("./assets/piano/piano.bmf"));
  s_piano->position = vec3_new(1, 5, 0);
  
  static_object* s_dino = entity_new("dino", static_object);
  s_dino->renderable = asset_hndl_new(P("./assets/dino/dino.bmf"));
  s_dino->scale = vec3_new(4 ,4, 4);
  
  animated_object* a_imrod = entity_new("imrod", animated_object);
  animated_object_load_skeleton(a_imrod, asset_hndl_new(P("./assets/imrod/imrod.skl")));
  
  a_imrod->renderable = asset_hndl_new(P("./assets/imrod/imrod.bmf"));
  a_imrod->animation = asset_hndl_new(P("./assets/imrod/imrod.ani"));
  a_imrod->rotation = quat_rotation_y(1.57);
  a_imrod->scale = vec3_new(1.25, 1.25, 1.25);
  
  /* Put some text on the screen */
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  
  ui_button* object = ui_elem_new("object", ui_button);
  ui_button_move(object, vec2_new(10, graphics_viewport_height() - 70));
  ui_button_resize(object, vec2_new(60,25));
  ui_button_set_label(object, "Object");
  ui_button_disable(object);
  
  ui_button* piano = ui_elem_new("piano", ui_button);
  ui_button_move(piano, vec2_new(80, graphics_viewport_height() - 70));
  ui_button_resize(piano, vec2_new(50,25));
  ui_button_set_label(piano, "Piano");
  
  ui_button* cello = ui_elem_new("cello", ui_button);
  ui_button_move(cello, vec2_new(140, graphics_viewport_height() - 70));
  ui_button_resize(cello, vec2_new(50,25));
  ui_button_set_label(cello, "Cello");
  
  ui_button* imrod = ui_elem_new("imrod", ui_button);
  ui_button_move(imrod, vec2_new(200, graphics_viewport_height() - 70));
  ui_button_resize(imrod, vec2_new(50,25));
  ui_button_set_label(imrod, "Imrod");
  
  ui_button* dino = ui_elem_new("dino", ui_button);
  ui_button_move(dino, vec2_new(260, graphics_viewport_height() - 70));
  ui_button_resize(dino, vec2_new(40,25));
  ui_button_set_label(dino, "Dino");
  
  void onclick_piano() { object_id = 0; }
  void onclick_cello() { object_id = 1; }
  void onclick_imrod() { object_id = 2; }
  void onclick_dino()  { object_id = 3; }
  
  ui_button_set_onclick(piano, onclick_piano);
  ui_button_set_onclick(cello, onclick_cello);
  ui_button_set_onclick(imrod, onclick_imrod);
  ui_button_set_onclick(dino, onclick_dino);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(25.0, 25.0, 10.0);
  cam->target = vec3_new(0, 7, 0);
  
  /* Renderer Setup */
  
  dr = deferred_renderer_new(asset_hndl_new_load(P("./assets/graphics.cfg")));
  deferred_renderer_set_camera(dr, cam);
  deferred_renderer_set_tod(dr, 0.15, 0);
  deferred_renderer_set_skydome_enabled(dr, false);
  
  SDL_GL_CheckError();
}

void renderers_event(SDL_Event event) {
  
  camera* cam = entity_get("camera");
  camera_control_orbit(cam, event);
  
}

void renderers_update() {
  
  camera* cam = entity_get("camera");
  
  camera_control_joyorbit(cam, frame_time());

  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
  animated_object* imrod = entity_get("imrod");
  animated_object_update(imrod, frame_time() * 0.25);
  
}

void renderers_render() {
  
  deferred_renderer_add(dr, render_object_static(entity_get("podium")));

  switch (object_id) {
    case 0: deferred_renderer_add(dr, render_object_static(entity_get("piano"))); break;
    case 1: deferred_renderer_add(dr, render_object_static(entity_get("cello"))); break;
    case 2: deferred_renderer_add(dr, render_object_animated(entity_get("imrod"))); break;
    case 3: deferred_renderer_add(dr, render_object_static(entity_get("dino"))); break;
  }
  
  deferred_renderer_render(dr);
  
}

void renderers_finish() {
  deferred_renderer_delete(dr);
}

int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  corange_init("../../assets_core");
  
  renderers_init();
  
  int running = 1;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN: break;
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINTSCREEN) { graphics_viewport_screenshot(); }
        if ((event.key.keysym.sym == SDLK_r) &&
            (event.key.keysym.mod == KMOD_LCTRL)) {
              asset_reload_all();
          }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      renderers_event(event);
      ui_event(event);
    }
    
    renderers_update();
    ui_update();
    
    renderers_render();
    ui_render();
    graphics_swap(); 
    
    frame_end();
  }
  
  renderers_finish();
  
  corange_finish();
  
  return 0;
}
