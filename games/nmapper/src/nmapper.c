#include "corange.h"

static material* nmapper_mat;
static float* strength;

static char* strength_string;
static char* fov_string;

static ui_text* txt_strength;
static ui_text* txt_fov;

static renderable* r_cello;
static renderable* r_torus;

void nmapper_init() {
  
  viewport_set_multisamples(8);
  viewport_set_width(512);
  viewport_set_height(512);
  
  camera* cam = entity_new("camera", entity_type_camera);
  cam->position = v3(20.0, 0.0, 0.0);
  cam->target = v3_zero();
  
  light* sun = entity_new("sun", entity_type_light);
  sun->position = v3(30,43,-26);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  light_set_type(sun, light_type_spot);
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  
  load_folder("/resources/");
  load_folder("/resources/meshes/");
  load_folder("/resources/textures/");
  
  nmapper_mat = asset_get("/resources/nmapper.mat");
  
  texture* t_cello = asset_get("/resources/textures/cello_nm.dds");
  texture* t_blank = asset_get("/resources/textures/blank_nm.dds");
  
  model* m_cello = asset_get("/resources/meshes/cello.obj");
  model* m_torus = asset_get("/resources/meshes/torus.obj");
 
  r_cello = renderable_new(m_cello);
  renderable_set_material(r_cello, nmapper_mat);
  entity_add("cello", entity_type_static, static_object_new(r_cello));
  
  r_torus = renderable_new(m_torus);
  renderable_set_material(r_torus, nmapper_mat);
  entity_add("torus", entity_type_static, static_object_new(r_torus));
  
  strength = malloc(sizeof(float));
  *strength = 1.0f;
  
  strength_string = malloc(128);
  fov_string = malloc(128);
  strcpy(strength_string,"");
  strcpy(fov_string,"");
  
  font* console_font = asset_get("./engine/fonts/console_font.fnt");
  
  txt_strength = ui_text_new(strength_string, console_font);
  txt_strength->position = v2(10, 10);
  txt_strength->scale = v2(1.0,1.0);
  txt_strength->color = v4(1,1,1,1);
  ui_text_update(txt_strength);
  
  txt_fov = ui_text_new(fov_string, console_font);
  txt_fov->position = v2(10, 30);
  txt_fov->scale = v2(1.0,1.0);
  txt_fov->color = v4(1,1,1,1);
  ui_text_update(txt_fov);
  
  material_set_property(nmapper_mat, "bump_map", t_cello, mat_type_texture);
  material_set_property(nmapper_mat, "strength", strength, mat_type_float);
  
  glClearColor(0.5f, 0.5f, 1.0f, 0.0f);

}

static int mouse_down;
static int mouse_x;
static int mouse_y;

void nmapper_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  #define PI 3.14159265

  sprintf(strength_string, "Strength: %.2f", *strength);
  sprintf(fov_string, "FOV: %.2f degrees", cam->fov * (180.0 / PI) );

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  } 

  mouse_x = 0;
  mouse_y = 0;
  
  ui_text_update_string(txt_strength, strength_string);
  ui_text_update_string(txt_fov, fov_string);
  
}

void nmapper_render() {

  static_object* s_cello = entity_get("cello");

  forward_renderer_begin();
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  forward_renderer_render_static(s_cello);
  forward_renderer_end();
  
  ui_text_render(txt_strength);
  ui_text_render(txt_fov);

}

void nmapper_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_q) {
      cam->fov -= 0.01;
      cam->fov = cam->fov < 0.78 ? 0.78 : cam->fov;
      return;
    }
    
    if (event.key.keysym.sym == SDLK_w) {
      cam->fov += 0.01;
      cam->fov = cam->fov > 1.22 ? 1.22 : cam->fov;
      return;
    }
        
    if (event.key.keysym.sym == SDLK_KP_PLUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength += 0.075f;
      *strength = *strength > 2.0f ? 2.0f : *strength;
      return;
    }
    if (event.key.keysym.sym == SDLK_KP_MINUS) {
      float* strength = material_get_property(nmapper_mat, "strength");
      *strength -= 0.075f;
      *strength = *strength < 0.0f ? 0.0f : *strength;
      return;
    }
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

void nmapper_finish() {

  free(strength);

  ui_text_delete(txt_strength);
  ui_text_delete(txt_fov);
  
  renderable_delete(r_cello);
  renderable_delete(r_torus);
  
  forward_renderer_finish();

}