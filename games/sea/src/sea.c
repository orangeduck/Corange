
#include "corange.h"

static camera* cam;
static light* sun;

static model* seaplane;
static material* seaplane_mat;
static renderable* r_seaplane;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static int wireframe = 0;

static texture* noise1;
static texture* noise2;
static texture* noise3;
static texture* noise4;
static texture* noise5;

static texture* skydome;

static texture* water_calm;
static texture* water_foam;

void sea_init() {

  viewport_set_vsync(1);

  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  sun = light_new_type( v3(30,43,-26), light_type_spot );
  
  sun->ambient_color = v3(0.749, 0.855, 0.902);
  sun->diffuse_color = v3(1.0, 0.875, 0.573);
  
  shadow_mapper_init(sun);  

  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    
  load_folder("/resources/");
    
  noise1 = asset_get("/resources/noise1.dds");
  noise2 = asset_get("/resources/noise2.dds");
  noise3 = asset_get("/resources/noise3.dds");
  noise4 = asset_get("/resources/noise4.dds");
  noise5 = asset_get("/resources/noise5.dds");
  
  skydome = asset_get("/resources/skybox_cloud_10.dds");

  water_calm = asset_get("/resources/water_calm.dds");
  water_foam = asset_get("/resources/water_foam.dds");
  
  seaplane = asset_get("/resources/seaplane.obj");
  seaplane_mat = asset_get("/resources/seaplane.mat");
  
  material_set_property(seaplane_mat, "tex_noise1", noise1, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise2", noise2, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise3", noise3, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise4", noise4, mat_type_texture);
  material_set_property(seaplane_mat, "tex_noise5", noise5, mat_type_texture);
  
  material_set_property(seaplane_mat, "tex_skybox", skydome, mat_type_texture);

  material_set_property(seaplane_mat, "tex_calm_water", water_calm, mat_type_texture);
  material_set_property(seaplane_mat, "tex_foam_water", water_foam, mat_type_texture);
  
  r_seaplane = renderable_new("seaplane");
  renderable_add_model(r_seaplane, seaplane);
  renderable_set_material(r_seaplane, seaplane_mat);
    
}

void sea_update() {

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

  forward_renderer_begin();
  
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  forward_renderer_render_renderable(r_seaplane);
  
  if(wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  
  forward_renderer_end();
  
}

void sea_event(SDL_Event event) {

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
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }

}

void sea_finish() {

  renderable_delete(r_seaplane);

  camera_delete(cam);
  light_delete(sun);
  
  forward_renderer_finish();

}