#include "corange.h"

#include "spline.h"

#define CUBE_SIZE 10

static int mouse_x;
static int mouse_y;
static int mouse_down;

GLuint cube_positions_buffer;
GLuint cube_colors_buffer;

void lut_gen_init() {


  asset_manager_handler("acv", acv_load_file, color_curves_delete);
  
  load_folder("./input/");
  
  color_curves_write_lut(asset_get_as("./input/test.acv", color_curves), "./output/test.lut");
  color_curves_write_lut(asset_get_as("./input/test2.acv", color_curves), "./output/test2.lut");
  color_curves_write_lut(asset_get_as("./input/bluey.acv", color_curves), "./output/bluey.lut");
  
  load_folder("./output/");
  load_folder("./screenshots/");
  load_folder("./shaders/");
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(CUBE_SIZE * 1.5, CUBE_SIZE * 1.5, CUBE_SIZE * 1.5);
  cam->target = v3(CUBE_SIZE/2, CUBE_SIZE/2, CUBE_SIZE/2);
  
  glClearColor(0.5, 0.5, 0.5, 1.0);
  glClearDepth(1.0);

  color_curves* curves = asset_get("./input/test2.acv");
  
  vector3* positions = malloc(sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE);
  vector3* colors = malloc(sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE);
  
  int x, y, z;
  for(x = 0; x < CUBE_SIZE; x++)
  for(y = 0; y < CUBE_SIZE; y++)
  for(z = 0; z < CUBE_SIZE; z++) {
    
    int index = x + (CUBE_SIZE * y) + (CUBE_SIZE * CUBE_SIZE * z);
    
    positions[index] = v3(x, y, z);
    colors[index] = v3_div(v3(x, y, z), CUBE_SIZE);
    colors[index] = color_curves_map(curves, colors[index]);
  }
  
  glGenBuffers(1, &cube_positions_buffer);
  glGenBuffers(1, &cube_colors_buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE, positions, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_colors_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE, colors, GL_STATIC_DRAW);
  
  free(positions);
  free(colors);
  
}

void lut_gen_finish() {
  
  glDeleteBuffers(1, &cube_positions_buffer);
  glDeleteBuffers(1, &cube_colors_buffer);
  
}


static float proj_matrix[16];
static float view_matrix[16];

void lut_gen_render() {
  
  glViewport(400, 0, 400, 300);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  camera* cam = entity_get("camera");
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, viewport_ratio() );
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
  glEnable(GL_DEPTH_TEST);
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_positions_buffer);  
  glVertexPointer(3, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_colors_buffer);
  glColorPointer(3, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_COLOR_ARRAY);
  
  glEnable(GL_POINT_SMOOTH);
  glPointSize(3.0);
  
    glDrawArrays(GL_POINTS, 0, CUBE_SIZE * CUBE_SIZE * CUBE_SIZE);
  
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glDisable(GL_POINT_SMOOTH);
  glPointSize(1.0);
  glDisable(GL_DEPTH_TEST);
  
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.0);
  
  glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0); glVertex3f(0.0, 0.0, 0.0); glVertex3f(CUBE_SIZE, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0); glVertex3f(0.0, 0.0, 0.0); glVertex3f(0.0, CUBE_SIZE, 0.0);
    glColor3f(0.0, 0.0, 1.0); glVertex3f(0.0, 0.0, 0.0); glVertex3f(0.0, 0.0, CUBE_SIZE);
  glEnd();
  
  glLineWidth(1.0);
  glDisable(GL_LINE_SMOOTH);
  
  glColor3f(1.0, 1.0, 1.0);
  
  glViewport(0, 0, viewport_width(), viewport_height());
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  shader_program* prog = asset_get("./shaders/basic_lut.prog");
  
  glUseProgram(*prog);
  
  glUniform1i(glGetUniformLocation(*prog, "diffuse"), 0);
  glActiveTexture(GL_TEXTURE0 + 0 );
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *asset_get_as("./screenshots/shot1.dds", texture) );
  
  glUniform1i(glGetUniformLocation(*prog, "lut"), 1);
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_3D, *asset_get_as("./output/identity.lut", texture));
  glEnable(GL_TEXTURE_3D);
  
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(viewport_width() - 400, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(viewport_width(), 0, 0);
    glTexCoord2f(1, 1); glVertex3f(viewport_width(), 300, 0);
    glTexCoord2f(0, 1); glVertex3f(viewport_width() - 400, 300, 0);
  glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_3D);
  
  glUseProgram(0);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void lut_gen_update() {
  camera* cam = entity_get("camera");

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
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets/");
  
  lut_gen_init();
  
  bool running = true;
  while(running) {
    frame_begin();
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      
      switch(event.type){
      case SDL_KEYDOWN:
        break;
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_WHEELUP) {
          camera* cam = entity_get("camera");
          cam->position = v3_sub(cam->position, v3_normalize(cam->position));
        }
        if (event.button.button == SDL_BUTTON_WHEELDOWN) {
          camera* cam = entity_get("camera");
          cam->position = v3_add(cam->position, v3_normalize(cam->position));
        }
        break;
      case SDL_MOUSEMOTION:
        if ((event.motion.x > 400) && (event.motion.y > 300)) {
          mouse_x = event.motion.xrel;
          mouse_y = event.motion.yrel;
        }
        break;
      }
      
      ui_event(event);
      
    }
    
    lut_gen_update();
    ui_update();
    
    lut_gen_render();
    ui_render();
    
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }  
  
  corange_finish();
  
  return 0;
}
