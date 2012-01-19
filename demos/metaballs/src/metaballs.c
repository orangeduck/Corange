#include "particles.h"
#include "volume_renderer.h"
#include "marching_cubes.h"
#include "kernel.h"

#include "metaballs.h"

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

GLuint billboard_positions;
GLuint billboard_uvs;
const int max_particles = 1024;

void metaballs_init() {
  
  kernels_init_with_opengl();
  
  asset_manager_handler("cl", cl_load_file, kernel_program_delete);
  
  load_folder("./kernels/");
  
  particles_init();
  
  load_folder("./resources/floor/");
  load_folder("./resources/particles/");
  load_folder("./resources/shaders/");
   
  material* floor_mat = asset_get("./resources/floor/floor.mat");
  
  renderable* r_floor = asset_get("./resources/floor/floor.obj");
  renderable_set_material(r_floor, floor_mat);
  
  entity_add("floor", static_object, static_object_new(r_floor));
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(20.0, 20.0, 0.0);
  //cam->target = v3(0.0, 5.0, 0.0);
  cam->target = v3(0.0, 0.0, 0.0);
  
  light* sun = entity_new("sun", light);
  sun->position = v3(30,20,-26);
  sun->ambient_color = v3(0.5, 0.5, 0.5);
  sun->diffuse_color = v3(0.75, 0.75, 0.75);
  light_set_type(sun, light_type_spot);  
  
  shadow_mapper_init(sun);  
  
  forward_renderer_init();
  forward_renderer_set_camera(cam);
  forward_renderer_set_light(sun);
  forward_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
  
  vector3* temp_pos = malloc(sizeof(vector3) * 4 * max_particles);
  int i = 0;
  while (i < 4 * max_particles) {
    temp_pos[i]   = v3(-1, -1, 0);
    temp_pos[i+1] = v3(-1,  1, 0);
    temp_pos[i+2] = v3( 1,  1, 0);
    temp_pos[i+3] = v3( 1, -1, 0);
    i += 4;
  }
  
  glGenBuffers(1, &billboard_positions);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_positions);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * 4 * max_particles, temp_pos, GL_DYNAMIC_COPY);
  
  free(temp_pos);
  
  vector2* temp_uvs = malloc(sizeof(vector2) * 4 * max_particles);
  i = 0;
  while (i < 4 * max_particles) {
    temp_uvs[i]   = v2(0, 0);
    temp_uvs[i+1] = v2(0, 1);
    temp_uvs[i+2] = v2(1, 1);
    temp_uvs[i+3] = v2(1, 0);
    i += 4;
  }
  
  glGenBuffers(1, &billboard_uvs);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_uvs);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector2) * 4 * max_particles, temp_uvs, GL_DYNAMIC_COPY);
  
  free(temp_uvs);
  
  ui_text* ui_framerate = ui_elem_new("ui_framerate", ui_text);
  ui_framerate->position = v2( 20, 20 );
  ui_framerate->scale = v2(1,1);
  ui_framerate->color = v4(1,1,1,1);
  ui_text_update_string(ui_framerate, "framerate");
  
  ui_rectangle* ui_box = ui_elem_new("ui_box", ui_rectangle);
  ui_box->top_left = v2(15, 15);
  ui_box->bottom_right = v2(40, 40);
  ui_box->color = v4_black();
  ui_box->border_size = 2;
  ui_box->border_color = v4_white();
  
  //volume_renderer_init();
  //volume_renderer_set_camera(cam);
  //volume_renderer_set_light(sun);
  
  //pos1 = v3(0, 10, 0);
  //pos2 = v3(0, 5, 0);
  
  marching_cubes_init();
  
}

void metaballs_update() {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * frame_time() * 0.25;
    float a2 = (float)mouse_y * frame_time() * 0.25;
    
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

  particles_update(frame_time() / 10);
  
  ui_text* ui_framerate = ui_elem_get("ui_framerate");
  ui_text_update_string(ui_framerate, frame_rate_string());
  
  marching_cubes_clear();
  //marching_cubes_point(1, 1, 1, 1.0);
  marching_cubes_metaball(5, 5, 5, 3);
  marching_cubes_update();
  
}

static float proj_matrix[16];
static float view_matrix[16];

void metaballs_render() {

  static_object* s_floor = entity_get("floor");
  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");
  
  shader_program* particle_program = asset_get("./resources/shaders/particles.prog");
  texture* particle_texture = asset_get("./resources/particles/white.dds");

  shadow_mapper_begin();
  shadow_mapper_render_static(s_floor);
  shadow_mapper_end();

  
  forward_renderer_begin();
  
    glClearColor(0.75f, 0.75f, 0.75f, 0.75f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    //forward_renderer_render_static(s_floor);

    /*
    
    glUseProgram(*particle_program);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glUniform1i(glGetUniformLocation(*particle_program, "particle_texture"), 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, *particle_texture);
    
    m44_to_array(camera_proj_matrix(cam, viewport_ratio()), proj_matrix);
    GLint proj_matrix_u = glGetUniformLocation(*particle_program, "proj_matrix");
    glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
    
    m44_to_array(camera_view_matrix(cam), view_matrix);
    GLint view_matrix_u = glGetUniformLocation(*particle_program, "view_matrix");
    glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
    
    GLuint particle_position = glGetAttribLocation(*particle_program, "particle_position");
    glEnableVertexAttribArray(particle_position);
    GLuint particle_velocity = glGetAttribLocation(*particle_program, "particle_velocity");
    glEnableVertexAttribArray(particle_velocity);
    
    glBindBuffer(GL_ARRAY_BUFFER, particle_positions_buffer());
    glVertexAttribPointer(particle_position, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, particle_velocities_buffer());
    glVertexAttribPointer(particle_velocity, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, billboard_positions);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, billboard_uvs);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glDrawArrays(GL_QUADS, 0, particles_count() * 4);
  
    glDisableVertexAttribArray(particle_position);
    glDisableVertexAttribArray(particle_velocity);
  
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glUseProgram(0);
  
    */
  
    marching_cubes_render();
  
  forward_renderer_end();
  
  /*
  volume_renderer_begin();
  volume_renderer_render_point(sun->position, v3_blue());
  
  srand(time(NULL));
  
  vector4* positions = particle_positions();
  int i;
  for(i = 0; i < 25; i++) {
    vector3 pos = v3(positions[i].x, positions[i].y, positions[i].z);
    pos = v3_mul(pos, 50);
    //vector3 col = v3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
    vector3 col = v3(1, 0, 0);
    volume_renderer_render_metaball(pos, col);
  }
  volume_renderer_end();
  */
  
  ui_render();
  
  SDL_GL_SwapBuffers();
  
}

void metaballs_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  light* sun = entity_get("sun");

  switch(event.type){

  case SDL_KEYUP:
    
    if (event.key.keysym.sym == SDLK_SPACE) { 
      particles_reset();
    }
    
    /*
    if(event.key.keysym.sym == SDLK_UP) {
      pos1 = v3_add(pos1, v3(0, 0.1, 0));
    }
    if(event.key.keysym.sym == SDLK_DOWN) {
      pos1 = v3_sub(pos1, v3(0, 0.1, 0));
    }
    */
    
  break;
  
  case SDL_MOUSEBUTTONUP:
    
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

void metaballs_finish() {
  
  forward_renderer_finish();
  
  shadow_mapper_finish();
  
  particles_finish();

  glDeleteBuffers(1, &billboard_positions);
  glDeleteBuffers(1, &billboard_uvs);
  
  //volume_renderer_finish();
  marching_cubes_finish();
  
  kernels_finish();
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  metaballs_init();
  
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
      metaballs_event(event);
    }
    
    metaballs_update();
    
    metaballs_render();
    
    frame_end();
  }
  
  metaballs_finish();
  
  corange_finish();
  
  return 0;
}
