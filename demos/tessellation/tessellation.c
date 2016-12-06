#include "corange.h"

static float tess_level_inner = 3;
static float tess_level_outer = 3;

static GLsizei index_count;
static GLuint positions_buffer;
static GLuint index_buffer;

static void create_mesh() {

  const int faces[] = {
    2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 1, 5, 0,
    11, 6, 7, 11, 7, 8, 11, 8, 9, 11, 9, 10, 11, 10, 6,
    1, 2, 6, 2, 3, 7, 3, 4, 8, 4, 5, 9, 5, 1, 10,
    2,  7, 6, 3, 8, 7, 4, 9, 8, 5, 10, 9, 1, 6, 10};

  const float verts[] = {
    0.000f,  0.000f,  1.000f, 0.894f,  0.000f,  0.447f,
    0.276f,  0.851f,  0.447f, -0.724f,  0.526f,  0.447f,
    -0.724f, -0.526f,  0.447f,  0.276f, -0.851f,  0.447f,
    0.724f,  0.526f, -0.447f, -0.276f,  0.851f, -0.447f,
    -0.894f,  0.000f, -0.447f, -0.276f, -0.851f, -0.447f,
    0.724f, -0.526f, -0.447f, 0.000f,  0.000f, -1.000f };

  index_count = sizeof(faces) / sizeof(int);
  
  glGenBuffers(1, &positions_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  
  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);
}

static bool tessellation_supported;

void tessellation_init() {

  graphics_viewport_set_title("Tessellation");
  graphics_viewport_set_size(1280, 720);
  
  tessellation_supported = SDL_GL_ExtensionPresent("GL_ARB_tessellation_shader");
  
  if (!tessellation_supported) {
    ui_button* not_supported = ui_elem_new("not_supported", ui_button);
    ui_button_move(not_supported, vec2_new(graphics_viewport_width()/2 - 205, graphics_viewport_height()/2 - 12));
    ui_button_resize(not_supported, vec2_new(410, 25));
    ui_button_set_label(not_supported, "Sorry your graphics card doesn't support tessellation!");
    return;
  }
  
  folder_load(P("./"));
  
  camera* cam = entity_new("cam", camera);
  cam->position = vec3_new(2,2,2);
  
  light* sun = entity_new("sun", light);
  
  create_mesh();
  
  ui_button* controls = ui_elem_new("controls", ui_button);
  ui_button_move(controls, vec2_new(10,10));
  ui_button_resize(controls, vec2_new(300, 25));
  ui_button_set_label(controls, "Up/Down Arrows to adjust Tessellation.");
  
}

void tesselation_event(SDL_Event event) {
  
  //camera* cam = entity_get("cam");
  //light* sun = entity_get("sun");
  //camera_control_orbit(cam, event);
  
  switch(event.type){
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_UP) { tess_level_inner++; tess_level_outer++; }
    if (event.key.keysym.sym == SDLK_DOWN) { tess_level_inner = max(tess_level_inner-1, 1); tess_level_outer = max(tess_level_outer-1, 1); }
  break;
  }
    
}

void tesselation_update() {
}

void tessellation_render() {
  
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (!tessellation_supported) { return; }
  
  light* sun = entity_get("sun");
  camera* cam = entity_get("cam");
  
  material* tess_mat = asset_get(P("./tessellation.mat"));
  shader_program* tess_shader = material_first_program(tess_mat);
  
  shader_program_enable(tess_shader);
  shader_program_set_float(tess_shader, "tess_level_inner", tess_level_inner);
  shader_program_set_float(tess_shader, "tess_level_outer", tess_level_outer);
  shader_program_set_mat4(tess_shader, "view", camera_view_matrix(cam));
  shader_program_set_mat4(tess_shader, "proj", camera_proj_matrix(cam));
  
  glEnable(GL_DEPTH_TEST);
     
    glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
    shader_program_enable_attribute(tess_shader, "Position", 3, 3, 0);
    
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glDrawElements(GL_PATCHES, index_count, GL_UNSIGNED_INT, 0);
  
    shader_program_disable_attribute(tess_shader, "Position");
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  glDisable(GL_DEPTH_TEST);
  
  shader_program_disable(tess_shader);
  
  SDL_GL_CheckError();
}


int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  
  corange_init("../../assets_core");
  
  tessellation_init();
  
  bool running = true;
  while(running) {
    frame_begin();
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINTSCREEN) { graphics_viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      break;
      }
      
      tesselation_event(event);
      ui_event(event);
      
    }
    
    tesselation_update();
    ui_update();
    
    tessellation_render();
    ui_render();
    
    graphics_swap();
    
    frame_end();
    
  }
  
  corange_finish();
  
  return 0;
}
