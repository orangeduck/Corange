#include <stdio.h>

#include "renderers.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static int object_id = 1;
static int use_deferred = 1;

static void swap_renderer() {
  
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
    
    ui_text* renderer_text = ui_elem_get("renderer_text");
    ui_text_update_string(renderer_text,"Forward Renderer");
    
    use_deferred = 0;
    
  } else {
    
    forward_renderer_finish();
    deferred_renderer_init();
    deferred_renderer_set_camera(cam);
    deferred_renderer_set_shadow_light(sun);
    deferred_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    deferred_renderer_add_light(sun);
    deferred_renderer_add_light(backlight);
    
    ui_text* renderer_text = ui_elem_get("renderer_text");
    ui_text_update_string(renderer_text,"Deferred Renderer");
    
    use_deferred = 1;
  }

}

static bool renderer_button_pressed = false;
static void switch_renderer_event(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      renderer_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (renderer_button_pressed) {
      renderer_button_pressed = false;
      rect->color = v4_black();
      swap_renderer();
    }
  }
}

static bool piano_button_pressed;
static void switch_object_piano(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      piano_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (piano_button_pressed) {
      piano_button_pressed = false;
      rect->color = v4_black();
      
      object_id = 1;
    }
  }
}

static bool cello_button_pressed;
static void switch_object_cello(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      cello_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (cello_button_pressed) {
      cello_button_pressed = false;
      rect->color = v4_black();
      
      object_id = 0;
    }
  }
}

static bool imrod_button_pressed;
static void switch_object_imrod(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      imrod_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (imrod_button_pressed) {
      imrod_button_pressed = false;
      rect->color = v4_black();
      
      object_id = 2;
    }
  }
}

static bool dino_button_pressed;
static void switch_object_dino(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      dino_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (dino_button_pressed) {
      dino_button_pressed = false;
      rect->color = v4_black();
      
      object_id = 4;
    }
  }
}

#define any_button_pressed (imrod_button_pressed || cello_button_pressed || piano_button_pressed || renderer_button_pressed || dino_button_pressed)

void renderers_init() {
  
  viewport_set_dimensions(1280, 720);

  load_folder("./resources/podium/");
  load_folder("./resources/cello/");
  load_folder("./resources/piano/");
  load_folder("./resources/imrod/");
  load_folder("./resources/pirate/");
  load_folder("./resources/dino/");
  
  renderable* r_podium = asset_get("./resources/podium/podium.obj");
  renderable_set_material(r_podium, asset_get("./resources/podium/podium.mat"));
  entity_add("podium", static_object, static_object_new(r_podium));
  
  renderable* r_cello = asset_get("./resources/cello/cello.obj");
  renderable_set_material(r_cello, asset_get("./resources/cello/cello.mat"));
  static_object* s_cello = static_object_new(r_cello);
  s_cello->position = v3(0, 3, 0);
  s_cello->rotation = v4_quaternion_mul(s_cello->rotation, v4_quaternion_yaw(-1.7));
  s_cello->scale = v3(0.6, 0.6, 0.6);
  entity_add("cello", static_object, s_cello);
  
  renderable* r_piano = asset_get("./resources/piano/piano.obj");
  renderable_set_material(r_piano, asset_get("./resources/piano/piano.mat"));
  static_object* s_piano = static_object_new(r_piano);
  s_piano->position = v3(1, 5, 0);
  entity_add("piano", static_object, s_piano);
  
  renderable* r_dino = asset_get("./resources/dino/dino.obj");
  renderable_set_multi_material(r_dino, asset_get("./resources/dino/dino.mmat"));
  static_object* s_dino = static_object_new(r_dino);
  s_dino->scale = v3(4,4,4);
  entity_add("dino", static_object, s_dino);
  
  renderable* r_imrod = asset_get("./resources/imrod/imrod.smd");
  material* mat_imrod = asset_get("./resources/imrod/imrod_animated.mat");  
  renderable_set_material(r_imrod, mat_imrod);
  
  renderable* r_pirate = asset_get("./resources/pirate/pirate.smd");
  material* mat_pirate = asset_get("./resources/pirate/pirate.mat");  
  renderable_set_material(r_pirate, mat_pirate);
  
  renderable* r_boots = asset_get("./resources/pirate/boots.smd");
  material* mat_boots = asset_get("./resources/pirate/boots.mat");  
  renderable_set_material(r_boots, mat_boots);
  
  skeleton* skel_imrod = asset_get("./resources/imrod/imrod.skl");
  skeleton* skel_pirate = asset_get("./resources/pirate/pirate.skl");
  animation* ani_stand = asset_get("./resources/pirate/stand.ani");
  animation* ani_cheer = asset_get("./resources/pirate/cheer.ani");
  
  animated_object* imrod = animated_object_new(r_imrod, skel_imrod);
  imrod->animation = ani_stand;
  imrod->position = v3(0, -1.5, 0);
  imrod->scale = v3(1.1, 1.1, 1.1);
  imrod->rotation = v4_quaternion_mul(imrod->rotation, v4_quaternion_roll(1.57));
  entity_add("imrod", animated_object, imrod);
  
  animated_object* pirate = animated_object_new(r_pirate, skel_pirate);
  pirate->animation = ani_cheer;
  pirate->position = v3(0, 0, 0);
  entity_add("pirate", animated_object, pirate);
  
  animated_object* boots = animated_object_new(r_boots, skel_pirate);
  boots->animation = ani_cheer;
  boots->position = v3(0, 0, 0);
  entity_add("boots", animated_object, boots);
  
  /* Put some text on the screen */
  
  ui_rectangle* framerate_rect = ui_elem_new("framerate_rect", ui_rectangle);
  framerate_rect->top_left = v2(10,10);
  framerate_rect->bottom_right = v2(45, 35);
  framerate_rect->color = v4_black();
  framerate_rect->border_color = v4_white();
  framerate_rect->border_size = 1;
  
  ui_text* framerate_text = ui_elem_new("framerate_text", ui_text);
  framerate_text->position = v2(15, 15);
  framerate_text->color = v4_white();
  ui_text_update_string(framerate_text, "framerate");
  
  ui_rectangle* renderer_rect = ui_elem_new("renderer_rect", ui_rectangle);
  renderer_rect->top_left = v2(55,10);
  renderer_rect->bottom_right = v2(210,35);
  renderer_rect->color = v4_black();
  renderer_rect->border_color = v4_white();
  renderer_rect->border_size = 1;
  
  ui_text* renderer_text = ui_elem_new("renderer_text", ui_text);
  renderer_text->position = v2(65, 15);
  renderer_text->color = v4_white();
  ui_text_update_string(renderer_text, "Deferred Renderer");
  
  /* Object Buttons */
  
  ui_rectangle* switch_rect = ui_elem_new("switch_rect", ui_rectangle);
  switch_rect->top_left = v2(10,45);
  switch_rect->bottom_right = v2(150,70);
  switch_rect->color = v4_black();
  switch_rect->border_color = v4_white();
  switch_rect->border_size = 1;
  
  ui_text* switch_text = ui_elem_new("switch_text", ui_text);
  switch_text->position = v2(15, 50);
  switch_text->color = v4_white();
  ui_text_update_string(switch_text, "Switch Renderer");
  
  ui_rectangle* piano_rect = ui_elem_new("piano_rect", ui_rectangle);
  piano_rect->top_left = v2(10,80);
  piano_rect->bottom_right = v2(60,105);
  piano_rect->color = v4_black();
  piano_rect->border_color = v4_white();
  piano_rect->border_size = 1;
  
  ui_text* piano_text = ui_elem_new("piano_text", ui_text);
  piano_text->position = v2(15,85);
  piano_text->color = v4_white();
  ui_text_update_string(piano_text, "Piano");
  
  ui_rectangle* cello_rect = ui_elem_new("cello_rect", ui_rectangle);
  cello_rect->top_left = v2(10,115);
  cello_rect->bottom_right = v2(60,140);
  cello_rect->color = v4_black();
  cello_rect->border_color = v4_white();
  cello_rect->border_size = 1;
  
  ui_text* cello_text = ui_elem_new("cello_text", ui_text);
  cello_text->position = v2(15,120);
  cello_text->color = v4_white();
  ui_text_update_string(cello_text, "Cello");
  
  ui_rectangle* imrod_rect = ui_elem_new("imrod_rect", ui_rectangle);
  imrod_rect->top_left = v2(10,150);
  imrod_rect->bottom_right = v2(60,175);
  imrod_rect->color = v4_black();
  imrod_rect->border_color = v4_white();
  imrod_rect->border_size = 1;
  
  ui_text* imrod_text = ui_elem_new("imrod_text", ui_text);
  imrod_text->position = v2(15,155);
  imrod_text->color = v4_white();
  ui_text_update_string(imrod_text, "Imrod");
  
  ui_rectangle* dino_rect = ui_elem_new("dino_rect", ui_rectangle);
  dino_rect->top_left = v2(10,185);
  dino_rect->bottom_right = v2(60,210);
  dino_rect->color = v4_black();
  dino_rect->border_color = v4_white();
  dino_rect->border_size = 1;
  
  ui_text* dino_text = ui_elem_new("dino_text", ui_text);
  dino_text->position = v2(15,190);
  dino_text->color = v4_white();
  ui_text_update_string(dino_text, "Dino");
  
  ui_elem_add_event("switch_rect", switch_renderer_event);
  
  ui_elem_add_event("piano_rect", switch_object_piano);
  ui_elem_add_event("cello_rect", switch_object_cello);
  ui_elem_add_event("imrod_rect", switch_object_imrod);
  ui_elem_add_event("dino_rect", switch_object_dino);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(25.0, 25.0, 10.0);
  cam->target = v3(0, 5, 0);
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_spot);
  sun->position = v3(20,23,16);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(1.0,  0.894, 0.811);
  sun->power = 4.5;
  
  light* backlight = entity_new("backlight", light);
  light_set_type(backlight, light_type_point);
  backlight->position = v3(-22,10,-13);
  backlight->ambient_color = v3(0.2, 0.2, 0.2);
  backlight->diffuse_color = v3(0.729, 0.729, 1.0);
  backlight->specular_color = v3(0.5, 0.5, 0.5);
  backlight->power = 2;
  
  /* Renderer Setup */
  
  shadow_mapper_init(sun);
  
  use_deferred = 1;
  swap_renderer();

}

light* selected_light = NULL;
static void select_light(int x, int y) {
  
  float x_clip =  ((float)x / viewport_width()) * 2 - 1;
  float y_clip = -(((float)y / viewport_height()) * 2 - 1);
  
  light* light_ents[10];
  int num_light_ents;
  entities_get(light_ents, &num_light_ents, light);
  
  camera* cam = entity_get("camera");
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, viewport_ratio() );
  
  selected_light = NULL;
  float range = 0.1;
  
  for(int i = 0; i < num_light_ents; i++) {
    
    light* l = light_ents[i];
    
    vector4 light_pos = v4(l->position.x, l->position.y, l->position.z, 1);
    light_pos = m44_mul_v4(viewm, light_pos);
    light_pos = m44_mul_v4(projm, light_pos);
    
    light_pos = v4_div(light_pos, light_pos.w);
    
    float distance = v2_dist(v2(light_pos.x, light_pos.y), v2(x_clip, y_clip));
    if (distance < range) {
      range = distance;
      selected_light = l;
      debug("Selected Light %i", i);
    }
  }
  
}

static bool g_down = false;

void renderers_event(SDL_Event event) {
  
  ui_event(event);
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  static_object* s_cello = entity_get("cello");

  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_g) {
      g_down = true;
    }
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_g) {
      g_down = false;
    }
  break;

  case SDL_MOUSEBUTTONDOWN:
    if (event.button.button == SDL_BUTTON_WHEELUP) {
      cam->position = v3_sub(cam->position, v3_normalize(cam->position));
    }
    if (event.button.button == SDL_BUTTON_WHEELDOWN) {
      cam->position = v3_add(cam->position, v3_normalize(cam->position));
    }
  break;
  
  case SDL_MOUSEBUTTONUP:
    
    if (event.button.button == SDL_BUTTON_RIGHT) {
      select_light(event.motion.x, event.motion.y);
    }
    
  break;
  
  case SDL_MOUSEMOTION:
    if (!any_button_pressed) {
      mouse_x = event.motion.xrel;
      mouse_y = event.motion.yrel;
    }
  break;
  }
    
}

void renderers_update() {
  
  ui_update();
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  }

  if (g_down && selected_light) {
    selected_light->position.x += (float)mouse_x * 0.1;
    selected_light->position.z += (float)mouse_y * 0.1;
  }
  
  mouse_x = 0;
  mouse_y = 0;
  
  ui_text* framerate_text = ui_elem_get("framerate_text");
  ui_text_update_string(framerate_text, frame_rate_string());
  
  animated_object* imrod = entity_get("imrod");
  animated_object* pirate = entity_get("pirate");
  animated_object* boots = entity_get("boots");
  animated_object_update(imrod, 0.5);
  animated_object_update(pirate, 0.5);
  animated_object_update(boots, 0.5);
}

void renderers_render() {

  light* sun = entity_get("sun");
  light* backlight = entity_get("backlight");

  static_object* s_podium = entity_get("podium");
  
  static_object* s_piano = entity_get("piano");
  static_object* s_cello = entity_get("cello");
  static_object* s_dino = entity_get("dino");
  
  animated_object* a_imrod = entity_get("imrod");
  animated_object* a_pirate = entity_get("pirate");
  animated_object* a_boots = entity_get("boots");
  
  shadow_mapper_begin();
  shadow_mapper_render_static(s_podium);
  if (object_id == 0) {
    shadow_mapper_render_static(s_cello);
  } else if (object_id == 1) { 
    shadow_mapper_render_static(s_piano);
  } else if (object_id == 2) {
    shadow_mapper_render_animated(a_imrod);
  } else if (object_id == 3) {
    shadow_mapper_render_animated(a_pirate);
    shadow_mapper_render_animated(a_boots);
  } else if (object_id == 4) {
    shadow_mapper_render_static(s_dino);
  }
  shadow_mapper_end();
  
  if (use_deferred) {
    
    deferred_renderer_begin();
    deferred_renderer_render_static(s_podium);
    
    if (object_id == 0) {
      deferred_renderer_render_static(s_cello);
    } else if (object_id == 1) {
      deferred_renderer_render_static(s_piano);
    } else if (object_id == 2) {
      deferred_renderer_render_animated(a_imrod);
    } else if (object_id == 3) {
      deferred_renderer_render_animated(a_pirate);
      deferred_renderer_render_animated(a_boots);
    } else if (object_id == 4) {
      deferred_renderer_render_static(s_dino);
    }
    
    deferred_renderer_render_light(sun);
    deferred_renderer_render_light(backlight);
    
    if (selected_light != NULL) {
      deferred_renderer_render_axis(m44_world(selected_light->position, v3_one(), v4_quaternion_id()));
    }
    
    deferred_renderer_end();
    
  } else {
  
    forward_renderer_begin();
    
    forward_renderer_render_static(s_podium);
    
    if (object_id == 0) {
      forward_renderer_render_static(s_cello);
    } else if (object_id == 1) {
      forward_renderer_render_static(s_piano);
    } else if (object_id == 2) {
      forward_renderer_render_animated(a_imrod);
    } else if (object_id == 3) {
      forward_renderer_render_animated(a_pirate);
      forward_renderer_render_animated(a_boots);
    } else if (object_id == 4) {
      forward_renderer_render_static(s_dino);
    }
    
    forward_renderer_render_light(sun);
    forward_renderer_render_light(backlight);
    
    if (selected_light != NULL) {
      forward_renderer_render_axis(m44_world(selected_light->position, v3_one(), v4_quaternion_id()));
    }
    
    forward_renderer_end();
  }
  
}

void renderers_finish() {
  
  if (use_deferred) {
    deferred_renderer_finish();
  } else {
    forward_renderer_finish();
  }

  shadow_mapper_finish();
  
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
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      renderers_event(event);
    }
    
    renderers_update();
    
    renderers_render();
    ui_render();
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }
  
  renderers_finish();
  
  corange_finish();
  
  return 0;
}
