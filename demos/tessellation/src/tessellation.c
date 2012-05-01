#include <time.h>

#include "corange.h"

#include "tessellation.h"

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

  index_count = sizeof(faces) / sizeof(float);
  
  glGenBuffers(1, &positions_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  
  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);
}

static bool tessellation_supported;

void tessellation_init() {

  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Tessellation");
  
  load_folder("./shaders/");
    
  glClearColor(0.25, 0.25, 0.25, 1.0);
  
  camera* cam = entity_new("cam", camera);
  cam->position = v3(2,2,2);
  
  light* sun = entity_new("sun", light);
  
  create_mesh();
  
  tessellation_supported = SDL_GL_ExtensionPresent("GL_ARB_tessellation_shader");
  
  if (!tessellation_supported) {
    ui_button* not_supported = ui_elem_new("not_supported", ui_button);
    ui_button_move(not_supported, v2(graphics_viewport_width()/2 - 205, graphics_viewport_height()/2 - 12));
    ui_button_resize(not_supported, v2(410, 25));
    ui_button_set_label(not_supported, "Sorry your graphics card doesn't support tessellation!");
  } else {
    ui_button* controls = ui_elem_new("controls", ui_button);
    ui_button_move(controls, v2(10,10));
    ui_button_resize(controls, v2(300, 25));
    ui_button_set_label(controls, "Up/Down Arrows to adjust Tessellation.");
  }
  
}

static float mouse_x = 0;
static float mouse_y = 0;

void tesselation_event(SDL_Event event) {
  
  camera* cam = entity_get("cam");
  light* sun = entity_get("sun");
  
  switch(event.type){
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_UP) { tess_level_inner++; tess_level_outer++; }
    if (event.key.keysym.sym == SDLK_DOWN) { tess_level_inner = max(tess_level_inner-1, 1); tess_level_outer = max(tess_level_outer-1, 1); }
  break;
  
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

void tesselation_update() {

  camera* cam = entity_get("cam");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if (keystate & SDL_BUTTON(1)) {
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  }
  
  mouse_x = 0;
  mouse_y = 0;
}

void tessellation_render() {
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (!tessellation_supported) { return; }
  
  light* sun = entity_get("sun");
  camera* cam = entity_get("cam");
  
  GLuint sp_handle = shader_program_handle(asset_get("./shaders/tessellation.prog"));
  
  glUseProgram(sp_handle);
  
  glUniform1f(glGetUniformLocation(sp_handle, "tess_level_inner"), tess_level_inner);
  glUniform1f(glGetUniformLocation(sp_handle, "tess_level_outer"), tess_level_outer);
  
  glUniform3f(glGetUniformLocation(sp_handle, "light_position"), sun->position.x, sun->position.y, sun->position.z);
  
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, graphics_viewport_ratio() );
  
  float viewm_f[16]; m44_to_array(viewm, viewm_f);
  float projm_f[16]; m44_to_array(projm, projm_f);
  
  glUniformMatrix4fv(glGetUniformLocation(sp_handle, "view"), 1, 0, viewm_f);
  glUniformMatrix4fv(glGetUniformLocation(sp_handle, "proj"), 1, 0, projm_f);
  
  glEnable(GL_DEPTH_TEST);
  
  glEnableVertexAttribArray(glGetAttribLocation(sp_handle, "Position"));
    
    glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
    glVertexAttribPointer(glGetAttribLocation(sp_handle, "Position"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glDrawElements(GL_PATCHES, index_count, GL_UNSIGNED_INT, 0);
  
  glDisableVertexAttribArray(glGetAttribLocation(sp_handle, "Position"));
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  glDisable(GL_DEPTH_TEST);
  
  glUseProgram(0);
  
}


int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
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
        if (event.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
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
    
    SDL_GL_SwapBuffers();
    
    frame_end();
    
  }
  
  corange_finish();
  
  return 0;
}
