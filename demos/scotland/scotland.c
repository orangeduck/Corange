
#include "corange.h"

static static_object** blocks;

static int WIDTH = 1024;
static int HEIGHT = 1024;
static int BLOCK_SIZE = 32;
static int X_NUM = 32; /* (1024 / 32) */
static int Y_NUM = 32; /* (1024 / 32) */

void vegetation_init() {
  
  blocks = malloc(X_NUM * Y_NUM * sizeof(static_object*));
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    
    static_object* block = entity_new(blockname, static_object);
    block->renderable = asset_hndl_null();
    
    blocks[x + y*X_NUM] = block;
  }

}

void vegetation_finish() {
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    
    static_object* block = entity_get(blockname);
    //renderable_delete(block->renderable);
    
    entity_delete(blockname);
  }
  
  free(blocks);
  
}

void vegetation_update() {

}

void vegetation_render() {
  
  camera* cam = entity_get("camera");
  
  vec2 pos = vec2_div(vec2_new(cam->position.x, cam->position.z), BLOCK_SIZE);
  pos = vec2_clamp(pos, 1, 1022);
  
  char blockname0[512]; char blockname1[512]; char blockname2[512];
  char blockname3[512]; char blockname4[512]; char blockname5[512];
  char blockname6[512]; char blockname7[512]; char blockname8[512];
  
  sprintf(blockname0, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y-1);
  sprintf(blockname1, "vegblock_%i_%i", (int)pos.x  , (int)pos.y-1);
  sprintf(blockname2, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y-1);
  sprintf(blockname3, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y  );
  sprintf(blockname4, "vegblock_%i_%i", (int)pos.x  , (int)pos.y  );
  sprintf(blockname5, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y  );
  sprintf(blockname6, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y+1);
  sprintf(blockname7, "vegblock_%i_%i", (int)pos.x  , (int)pos.y+1);
  sprintf(blockname8, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y+1);
  
  static_object* block0 = entity_get(blockname0);
  static_object* block1 = entity_get(blockname1);
  static_object* block2 = entity_get(blockname2);
  static_object* block3 = entity_get(blockname3);
  static_object* block4 = entity_get(blockname4);
  static_object* block5 = entity_get(blockname5);
  static_object* block6 = entity_get(blockname6);
  static_object* block7 = entity_get(blockname7);
  static_object* block8 = entity_get(blockname8);
  
  /*
  forward_renderer_render_static(block0);
  forward_renderer_render_static(block1);
  forward_renderer_render_static(block2);
  forward_renderer_render_static(block3);
  forward_renderer_render_static(block4);
  forward_renderer_render_static(block5);
  forward_renderer_render_static(block6);
  forward_renderer_render_static(block7);
  forward_renderer_render_static(block8);
  */
  
}

static float random_in_range(float bottom, float top) {
  float difference = top - bottom;
  float random = (float)rand() / RAND_MAX;
  return bottom + random * difference;
}

static void block_part_add_template(model* blockpart, model* template, int index) {
  
  for(int i = 0; i < blockpart->num_meshes; i++) {
    mesh* tm = template->meshes[i];
    mesh* bm = blockpart->meshes[i];
  
    int vert_off = index * tm->num_verts;
    int tri_off = index * tm->num_triangles * 3;
    
    for(int j = 0; j < tm->num_verts; j++) {
      bm->verticies[vert_off + j] = tm->verticies[j];
    }
    
    for(int j = 0; j < tm->num_triangles * 3; j++) {
      bm->triangles[tri_off + j] = tri_off + tm->triangles[j];
    }
    
  }
  
}

static void paint_vegetation(model* m) {
  
  for(int i = 0; i < m->num_meshes; i++) {
  
    mesh* me = m->meshes[i];
    for(int j = 0; j < me->num_verts; j++) {
      if (me->verticies[j].position.y < 0) {
        me->verticies[j].color = vec4_black();
      } else {
        me->verticies[j].color = vec4_white();
      }
    }
    
  }
  
}

void vegetation_add_type(terrain* t, renderable* r, float density) {
  
  model* template = renderable_to_model(r);
  model_scale(template, 1.5);
  
  paint_vegetation(template);
  
  srand(time(NULL));
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    static_object* block = entity_get(blockname);
    
    int count = density * BLOCK_SIZE * BLOCK_SIZE;
    
    model* block_part = model_new();
    block_part->num_meshes = template->num_meshes;
    block_part->meshes = realloc(block_part->meshes, sizeof(mesh*) * template->num_meshes);
    
    for(int i = 0; i < template->num_meshes; i++) {
      
      block_part->meshes[i] = mesh_new();
      
      mesh* bm = block_part->meshes[i];
      mesh* tm = template->meshes[i];
      
      bm->num_verts = tm->num_verts * count;
      bm->num_triangles = tm->num_triangles * count;
      
      bm->verticies = realloc(bm->verticies, sizeof(vertex) * bm->num_verts);
      bm->triangles = realloc(bm->triangles, sizeof(int) * bm->num_triangles * 3);
    }
    
    for(int i = 0; i < count; i++) {
      
      float pos_x = random_in_range(x * BLOCK_SIZE, (x+1) * BLOCK_SIZE);
      float pos_z = random_in_range(y * BLOCK_SIZE, (y+1) * BLOCK_SIZE);
      float pos_y = terrain_height(t, vec2_new(pos_x, pos_z));
      
      float scale = random_in_range(0.8, 1.5);
      float rotation = random_in_range(0, 6.282);
      
      mat4 world = mat4_world( vec3_new(pos_x, pos_y, pos_z), vec3_new(scale, scale, scale), quat_rotation_y(rotation) );
      mat4 inv_world = mat4_inverse(world);
      
      model_transform(template, world);
      block_part_add_template(block_part, template, i);
      model_transform(template, inv_world);
    }
    
    //renderable_add_model(block->renderable, block_part);
    //renderable_set_material(block->renderable, asset_get("./resources/vegetation/grass.mat"));
    model_delete(block_part);
  }
  
  model_delete(template);

}

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static bool wireframe = false;
static bool freecam = true;

static void toggle_wireframe(ui_button* b, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_button_contains_position(b, vec2_new(event.motion.x, event.motion.y))) {
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
    
    if (ui_button_contains_position(b, vec2_new(event.motion.x, event.motion.y))) {
      b->pressed = true;
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (b->pressed) {
      b->pressed = false;
      
      freecam = !freecam;
      
      camera* cam = entity_get("camera");
      landscape* world = entity_get("world");
      
      vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
      float height = terrain_height(asset_hndl_ptr(world->terrain), vec2_new(cam->position.x, cam->position.z));
      cam->position.y = height + 1;
      cam->target = vec3_add(cam->position, cam_dir);
    }
  }
}

static bool loading_resources = false;

static int load_resources(void* unused) {
  
  folder_load(P("./resources/terrain/"));
  folder_load(P("./resources/vegetation/"));
  
  static_object* skydome = entity_new("skydome", static_object);
  skydome->renderable = asset_hndl_new_load(P("./resources/terrain/skydome.obj"));
  ((renderable*)asset_hndl_ptr(skydome->renderable))->material = asset_hndl_new_load(P("$CORANGE/shaders/skydome.mat"));
  skydome->position = vec3_new(512, 0, 512);
  skydome->scale = vec3_new(1024, 1024, 1024);
  
  landscape* world = entity_new("world", landscape);
  world->terrain = asset_hndl_new_load(P("./resources/terrain/heightmap.raw"));
  world->normalmap = asset_hndl_new_load(P("./resources/terrain/normalsmap.dds"));
  world->colormap = asset_hndl_new_load(P("./resources/terrain/colormap.dds"));
  world->attributemap = asset_hndl_new_load(P("./resources/terrain/attributemap.dds"));

  landscape_set_textures(world, asset_hndl_new_load(P("./resources/terrain/surface.dds")), 
                                asset_hndl_new_load(P("./resources/terrain/surface_bump.dds")), 
                                asset_hndl_new_load(P("./resources/terrain/surface_far.dds")), 
                                asset_hndl_new_load(P("./resources/terrain/surface_far_bump.dds")));
  
  vegetation_init();
  //vegetation_add_type(asset_get("./resources/terrain/heightmap.raw"), 
  //                    asset_get("./resources/vegetation/grass.obj"), 
  //                    4.0);
  
  ui_button* loading = ui_elem_get("loading");
  ui_spinner* load_spinner = ui_elem_get("load_spinner");
  ui_button* framerate = ui_elem_get("framerate");
  ui_button* wireframe = ui_elem_get("wireframe");
  ui_button* freecam = ui_elem_get("freecam");
  loading->active = false;
  load_spinner->active = false;
  framerate->active = true;
  wireframe->active = true;
  freecam->active = true;
  
  loading_resources = false;
  return 1;
}

void scotland_init() {
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Scotland");
  
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
  framerate->active = false;
  
  ui_button* wireframe = ui_elem_new("wireframe", ui_button);
  ui_button_move(wireframe, vec2_new(50,10));
  ui_button_resize(wireframe, vec2_new(80,25));
  ui_button_set_label(wireframe, "wireframe");
  wireframe->active = false;
  
  ui_elem_add_event("wireframe", toggle_wireframe);
  
  ui_button* freecam = ui_elem_new("freecam", ui_button);
  ui_button_move(freecam, vec2_new(140,10));
  ui_button_resize(freecam, vec2_new(65,25));
  ui_button_set_label(freecam, "freecam");
  freecam->active = false;
  
  ui_elem_add_event("freecam", toggle_freecam);
  
  loading_resources = true;
  SDL_Thread* load_thread = SDL_GL_CreateThread(load_resources, NULL);
  
  /* New Camera and light */
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(512.0, 200.0, 512.0);
  cam->target =  vec3_new(0.0, 0.0, 0.0);
  
  light* sun = entity_new("sun", light);
  light_set_type(sun, light_type_sun);
  sun->position = vec3_new(0, 512, 0);
  sun->target = vec3_new(512, 0, 512);
  
  /* Renderer Setup */
  
  deferred_renderer_init();
  deferred_renderer_set_camera(cam);
  
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

static float sun_orbit = -1.5;

void scotland_update() {
  
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  static_object* skydome = entity_get("skydome");
  landscape* world = entity_get("world");
  
  sun_orbit += frame_time() * 0.01;
  
  sun->position.x = 512 + sin(sun_orbit) * 512;
  sun->position.y = cos(sun_orbit) * 512;
  sun->position.z = 512;
  sun->target = vec3_new(512, 0, 512);
  
  if (w_held || s_held) {
    
    vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
    float speed = 0.5;
    if (!freecam) speed = 0.05;
    if (w_held) {
      cam->position = vec3_add(cam->position, vec3_mul(cam_dir, speed));
    }
    if (s_held) {
      cam->position = vec3_sub(cam->position, vec3_mul(cam_dir, speed));
    }
    
    if (!freecam) {
      float height = terrain_height(asset_hndl_ptr(world->terrain), vec2_new(cam->position.x, cam->position.z));
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
  
  light* sun = entity_get("sun");
  camera* cam = entity_get("camera");
  landscape* world = entity_get("world");
  static_object* skydome = entity_get("skydome");
  
  //texture_write_to_file(shadow_mapper_depth_texture(), "shadow_depth.tga");
  
  forward_renderer_begin();

  if (!wireframe) {
    forward_renderer_render_static(skydome);
  }
  
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }  
  forward_renderer_render_landscape(world);
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  
  if (!wireframe) {
    vegetation_render();
  }
  
}

void scotland_finish() {

  vegetation_finish();
  
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
        if (event.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      if (loading_resources) {
        ui_event(event);
      } else {
        scotland_event(event);
        ui_event(event);
      }
      
    }
    
    if (loading_resources) {
      ui_update();
    } else {
      scotland_update();
      ui_update();
    }
    
    
    if (loading_resources) {
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT);
      ui_render();
    } else {
      scotland_render();
      ui_render();
    }
    
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }
  
  scotland_finish();
  
  corange_finish();
  
  return 0;
}
