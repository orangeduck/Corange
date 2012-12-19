#include "renderers.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static int object_id = 1;
static bool use_deferred = true;
deferred_renderer* dr = NULL;

static void swap_renderer() {
  
  /*
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  light* backlight = entity_get("backlight");
  
  if (use_deferred) {
    
    deferred_renderer_finish();
    
    forward_renderer_init();
    forward_renderer_set_camera(cam);
    forward_renderer_set_shadow_light(sun);
    forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    forward_renderer_add_light(sun);
    forward_renderer_add_light(backlight);
    
    use_deferred = false;
    SDL_GL_CheckError();
    
  } else {
    
    forward_renderer_finish();
    
    deferred_renderer_init();
    deferred_renderer_set_camera(cam);
    deferred_renderer_set_shadow_light(sun);
    deferred_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    deferred_renderer_add_light(sun);
    deferred_renderer_add_light(backlight);
    deferred_renderer_disable_skydome();
    
    use_deferred = true;
    
  }
  */

}

static vec3 ik_target;

void renderers_init() {
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Renderers");
  
  folder_load(P("./resources/podium/"));
  folder_load(P("./resources/cello/"));
  folder_load(P("./resources/piano/"));
  folder_load(P("./resources/imrod/"));
  folder_load(P("./resources/dino/"));
  
  asset_hndl r_podium = asset_hndl_new(P("./resources/podium/podium.obj"));
  ((renderable*)asset_hndl_ptr(r_podium))->material = asset_hndl_new(P("./resources/podium/podium.mat"));
  
  static_object* s_podium = entity_new("podium", static_object);
  s_podium->renderable = r_podium;
  
  asset_hndl r_cello = asset_hndl_new(P("./resources/cello/cello.obj"));
  ((renderable*)asset_hndl_ptr(r_cello))->material = asset_hndl_new(P("./resources/cello/cello.mat"));
  
  static_object* s_cello = entity_new("cello", static_object);
  s_cello->renderable = r_cello;
  s_cello->position = vec3_new(0, 3, 0);
  //s_cello->rotation = quaternion_mul(s_cello->rotation, quaternion_yaw(-1.7));
  s_cello->scale = vec3_new(0.6, 0.6, 0.6);
  
  asset_hndl r_piano = asset_hndl_new(P("./resources/piano/piano.obj"));
  ((renderable*)asset_hndl_ptr(r_piano))->material = asset_hndl_new(P("./resources/piano/piano.mat"));
  
  static_object* s_piano = entity_new("piano", static_object);
  s_piano->renderable = r_piano;
  s_piano->position = vec3_new(1, 5, 0);
  
  asset_hndl r_dino = asset_hndl_new(P("./resources/dino/dino.obj"));
  ((renderable*)asset_hndl_ptr(r_dino))->material = asset_hndl_new(P("./resources/dino/dino.mat"));
  
  static_object* s_dino = entity_new("dino", static_object);
  s_dino->renderable = r_dino;
  s_dino->scale = vec3_new(4,4,4);
  
  SDL_GL_CheckError();
  
  asset_hndl r_imrod = asset_hndl_new(P("./resources/imrod/imrod.smd"));
  ((renderable*)asset_hndl_ptr(r_imrod))->material = asset_hndl_new(P("./resources/imrod/imrod_animated.mat"));
  
  animated_object* a_imrod = entity_new("imrod", animated_object);
  animated_object_load_skeleton(a_imrod, asset_hndl_new(P("./resources/imrod/imrod.skl")));
  
  a_imrod->renderable = r_imrod;
  a_imrod->animation = asset_hndl_new(P("./resources/imrod/imrod.ani"));
  //a_imrod->rotation = quaternion_mul(a_imrod->rotation, quaternion_roll(1.57));
  
  /* Put some text on the screen */
  
  SDL_GL_CheckError();
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "FRAMERATE");
  ui_button_disable(framerate);
  
  ui_button* renderer = ui_elem_new("renderer", ui_button);
  ui_button_move(renderer, vec2_new(10, graphics_viewport_height() - 35));
  ui_button_resize(renderer, vec2_new(75,25));
  ui_button_set_label(renderer, "Renderer");
  ui_button_disable(renderer);
  
  ui_button* forward_renderer = ui_elem_new("forward_renderer", ui_button);
  ui_button_move(forward_renderer, vec2_new(95, graphics_viewport_height() - 35));
  ui_button_resize(forward_renderer, vec2_new(65,25));
  ui_button_set_label(forward_renderer, "Forward");
  
  ui_button* deferred_renderer = ui_elem_new("deferred_renderer", ui_button);
  ui_button_move(deferred_renderer, vec2_new(170, graphics_viewport_height() - 35));
  ui_button_resize(deferred_renderer, vec2_new(75,25));
  ui_button_set_label(deferred_renderer, "Deferred");
  
  void forward_render_onclick() { use_deferred = true; swap_renderer(); }
  void deferred_renderer_onclick() { use_deferred = false; swap_renderer(); }
  
  ui_button_set_onclick(forward_renderer, forward_render_onclick);
  ui_button_set_onclick(deferred_renderer, deferred_renderer_onclick);
  
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
  
  void onclick_piano() { object_id = 1; }
  void onclick_cello() { object_id = 0; }
  void onclick_imrod() { object_id = 2; }
  void onclick_dino() { object_id = 3; }
  
  ui_button_set_onclick(piano, onclick_piano);
  ui_button_set_onclick(cello, onclick_cello);
  ui_button_set_onclick(imrod, onclick_imrod);
  ui_button_set_onclick(dino, onclick_dino);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(25.0, 25.0, 10.0);
  cam->target = vec3_new(0, 5, 0);
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_spot);
  sun->position = vec3_new(20,23,16);
  sun->ambient_color = vec3_new(0.5, 0.5, 0.5);
  sun->diffuse_color = vec3_new(1.0,  0.894, 0.811);
  sun->specular_color = vec3_mul(vec3_new(1.0,  0.894, 0.811), 4);
  sun->power = 5;
  
  light* backlight = entity_new("backlight", light);
  light_set_type(backlight, light_type_point);
  backlight->position = vec3_new(-22,10,-13);
  backlight->ambient_color = vec3_new(0.2, 0.2, 0.2);
  backlight->diffuse_color = vec3_new(0.729, 0.729, 1.0);
  backlight->specular_color = vec3_mul(vec3_new(0.729, 0.729, 1.0), 1);
  backlight->power = 2;
  
  /* Renderer Setup */
    
  dr = deferred_renderer_new();
  deferred_renderer_set_camera(dr, cam);
  deferred_renderer_set_sun_light(dr, sun);
  
  ik_target = vec3_new(0.0, 0.5, -2.8);
  
  SDL_GL_CheckError();
}

light* selected_light = NULL;
static void select_light(int x, int y) {
  
  float x_clip =  ((float)x / graphics_viewport_width()) * 2 - 1;
  float y_clip = -(((float)y / graphics_viewport_height()) * 2 - 1);
  
  light* light_ents[10];
  int num_light_ents;
  entities_get(light_ents, &num_light_ents, light);
  
  camera* cam = entity_get("camera");
  mat4 viewm = camera_view_matrix(cam);
  mat4 projm = camera_proj_matrix(cam);
  
  selected_light = NULL;
  float range = 0.1;
  
  for(int i = 0; i < num_light_ents; i++) {
    
    light* l = light_ents[i];
    
    vec4 light_pos = vec4_new(l->position.x, l->position.y, l->position.z, 1);
    light_pos = mat4_mul_vec4(viewm, light_pos);
    light_pos = mat4_mul_vec4(projm, light_pos);
    
    light_pos = vec4_div(light_pos, light_pos.w);
    
    float distance = vec2_dist(vec2_new(light_pos.x, light_pos.y), vec2_new(x_clip, y_clip));
    if (distance < range) {
      range = distance;
      selected_light = l;
      debug("Selected Light %i", i);
    }
  }
  
}

static bool g_down = false;
static bool t_down = false;
static bool w_down = false;
static bool a_down = false;
static bool s_down = false;
static bool d_down = false;

void renderers_event(SDL_Event event) {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  static_object* s_cello = entity_get("cello");

  camera_control_orbit(cam, event);
  
  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_g) g_down = true;
    if (event.key.keysym.sym == SDLK_t) t_down = true;
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_g) g_down = false;
    if (event.key.keysym.sym == SDLK_t) t_down = false;
  break;
  
  case SDL_MOUSEBUTTONUP:
    if (event.button.button == SDL_BUTTON_RIGHT) {
      select_light(event.motion.x, event.motion.y);
    }
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }
    
}

void renderers_update() {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  camera_control_joyorbit(cam, frame_time());
  
  /*
  if (keystate & SDL_BUTTON(3)) {
    
    matrix_4x4 view = camera_view_matrix(cam);
    matrix_4x4 proj = camera_proj_matrix(cam, graphics_viewport_ratio());
    matrix_4x4 inv_view = m44_inverse(view);
    matrix_4x4 inv_proj = m44_inverse(proj);
    
    vec3 ik_screen = ik_target;
    ik_screen = m44_mul_vec3(view, ik_screen);
    ik_screen = m44_mul_vec3(proj, ik_screen);
    
    ik_screen.x +=  (float)mouse_x * 0.001;
    ik_screen.y += -(float)mouse_y * 0.001;
    
    vec3 ik_world = ik_screen;
    ik_world = m44_mul_vec3(inv_proj, ik_world);
    ik_world = m44_mul_vec3(inv_view, ik_world);
    
    ik_target = ik_world;
  }
  */
  
  if (g_down && selected_light) {
    vec3 move_dir = vec3_sub(cam->target, cam->position); move_dir.y = 0;
    
    vec3 move_forward = vec3_normalize(move_dir);
    vec3 move_left = vec3_cross(move_forward, vec3_new(0,1,0));
    
    vec3 move = vec3_add( vec3_mul(move_forward, -mouse_y), vec3_mul(move_left, mouse_x));
    
    selected_light->position.x += (float)move.x * 0.1;
    selected_light->position.z += (float)move.z * 0.1;
  }
  
  if (t_down && selected_light) {
    selected_light->position.y += (float)-mouse_y * 0.1;
  }
  
  mouse_x = 0;
  mouse_y = 0;
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
  animated_object* imrod = entity_get("imrod");
  animated_object_update(imrod, 0.1);
  
  /*
  bone* thigh_r = skeleton_bone_name(imrod->pose, "thigh_r");
  bone* foot_r = skeleton_bone_name(imrod->pose, "foot_r");
  matrix_4x4 inv_world = m44_inverse(m44_world(imrod->position, imrod->scale, imrod->rotation));
  
  vec3 local_target = m44_mul_vec3(inv_world, ik_target);
  
  inverse_kinematics_solve(thigh_r, foot_r, local_target);
  */
  
}

void renderers_render() {
  
  deferred_renderer_add(dr, render_object_static(entity_get("podium")));
  deferred_renderer_add(dr, render_object_static(entity_get("piano")));
  //deferred_renderer_add(dr, render_object_static(entity_get("cello")));
  //deferred_renderer_add(dr, render_object_static(entity_get("dino")));
  //deferred_renderer_add(dr, render_object_animated(entity_get("imrod")));
  deferred_renderer_add(dr, render_object_light(entity_get("sun")));
  deferred_renderer_add(dr, render_object_light(entity_get("backlight")));
  
  deferred_renderer_render(dr);
  
}

void renderers_finish() {
  deferred_renderer_delete(dr);
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
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
        if (event.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        if ((event.key.keysym.sym == SDLK_r) &&
            (event.key.keysym.mod == KMOD_LCTRL)) {
              asset_reload_all();
          }
        if ((event.key.keysym.sym == SDLK_t) &&
            (event.key.keysym.mod == KMOD_LCTRL)) {
              asset_reload_type(texture);
          }
        if ((event.key.keysym.sym == SDLK_s) &&
            (event.key.keysym.mod == KMOD_LCTRL)) {
              asset_reload_type(material);
              asset_reload_type(shader);
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
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }
  
  renderers_finish();
  
  corange_finish();
  
  return 0;
}
