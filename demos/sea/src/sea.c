
#include "sea.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static bool wireframe = false;

static bool wireframe_button_pressed = false;
static void switch_wireframe(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      wireframe_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (wireframe_button_pressed) {
      wireframe_button_pressed = false;
      rect->color = v4_black();
      
      if (wireframe == true) {
        wireframe = false;
      } else if (wireframe == false) {
        wireframe = true;
      }
      
    }
  }
}

void sea_init() {

  viewport_set_vsync(1);

  camera* cam = entity_new("camera", camera);
  cam->position = v3(30.0, 30.0, 30.0);
  cam->target = v3_zero();
  
  light* sun = entity_new("sun", light);
  sun->position = v3(30,43,-26);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  light_set_type(sun, light_type_spot);
  
  shadow_mapper_init(sun);  

  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  
  load_folder("./resources/");
    
  texture* noise1 = asset_get("./resources/noise1.dds");
  texture* noise2 = asset_get("./resources/noise2.dds");
  texture* noise3 = asset_get("./resources/noise3.dds");
  texture* noise4 = asset_get("./resources/noise4.dds");
  texture* noise5 = asset_get("./resources/noise5.dds");
  
  texture* skydome = asset_get("./resources/skybox_cloud_10.dds");

  texture* water_calm = asset_get("./resources/water_calm.dds");
  texture* water_foam = asset_get("./resources/water_foam.dds");
  
  material* seaplane_mat = asset_get("./resources/seaplane.mat");
  
  material_set_property(seaplane_mat, "tex_noise1", noise1, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise2", noise2, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise3", noise3, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise4", noise4, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise5", noise5, mat_type_texture);
  
  material_set_property(seaplane_mat, "tex_skybox", skydome, mat_type_texture);

  material_set_property(seaplane_mat, "tex_calm_water", water_calm, mat_type_texture);
  material_set_property(seaplane_mat, "tex_foam_water", water_foam, mat_type_texture);
  
  renderable* r_seaplane = asset_get("./resources/seaplane.obj");
  renderable_set_material(r_seaplane, seaplane_mat);
  
  entity_add("seaplane", static_object, static_object_new(r_seaplane));
  
  load_folder("./resources/skybox/");
  
  renderable* r_skybox = asset_get("./resources/skybox/skybox.obj");
  renderable_set_material(r_skybox, asset_get("./resources/skybox/skybox.mat"));
  static_object* s_skybox = static_object_new(r_skybox);
  s_skybox->recieve_shadows = false;
  entity_add("skybox", static_object, s_skybox);
  
  load_folder("./resources/corvette/");
  
  ui_rectangle* wireframe_rect = ui_elem_new("wireframe_rect", ui_rectangle);
  wireframe_rect->top_left = v2(10,10);
  wireframe_rect->bottom_right = v2(100, 35);
  wireframe_rect->color = v4_black();
  wireframe_rect->border_color = v4_white();
  wireframe_rect->border_size = 1;
  
  ui_elem_add_event("wireframe_rect", switch_wireframe);
  
  ui_text* wireframe_text = ui_elem_new("wireframe_text", ui_text);
  wireframe_text->position = v2(20, 15);
  wireframe_text->color = v4_white();
  ui_text_update_string(wireframe_text, "Wireframe");
}

void sea_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * 0.025;
    float a2 = (float)mouse_y * 0.025;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  }
  
  if(keystate & SDL_BUTTON(3)){
    sun->position.x += (float)mouse_y / 2;
    sun->position.z -= (float)mouse_x / 2;
  }

  mouse_x = 0;
  mouse_y = 0;
  
}

void sea_render() {

  static_object* s_seaplane = entity_get("seaplane");
  static_object* s_skybox = entity_get("skybox");

  forward_renderer_begin();
  
  forward_renderer_render_static(s_skybox);
  
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }  
  forward_renderer_render_static(s_seaplane);
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  
  forward_renderer_end();
  
}

void sea_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_w) { if(wireframe == 1){wireframe = 0;} else { wireframe = 1;} }
  
  case SDL_MOUSEBUTTONDOWN:

    if (event.button.button == SDL_BUTTON_WHEELUP) {
      cam->position = v3_sub(cam->position, v3_normalize(cam->position));
    }
    if (event.button.button == SDL_BUTTON_WHEELDOWN) {
      cam->position = v3_add(cam->position, v3_normalize(cam->position));
    }
    
  break;
  
  case SDL_MOUSEMOTION:
    if (!wireframe_button_pressed) {
      mouse_x = event.motion.xrel;
      mouse_y = event.motion.yrel;
    }
  break;
  }

}

void sea_finish() {
  forward_renderer_finish();
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  sea_init();
  
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
      sea_event(event);
      ui_event(event);
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
