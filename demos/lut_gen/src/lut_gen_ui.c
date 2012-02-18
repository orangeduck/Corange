#include "corange.h"

#include "spline.h"

#define CUBE_SIZE 8

static int mouse_x;
static int mouse_y;
static int mouse_down;

GLuint cube_positions_buffer;
GLuint cube_colors_buffer;

static color_curves* current_curves = NULL;
static texture* current_lut = NULL;

static void lut_gen_rebuild_cube() {
  
  vector3* positions = malloc(sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE);
  vector3* colors = malloc(sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE);
  
  int x, y, z;
  for(x = 0; x < CUBE_SIZE; x++)
  for(y = 0; y < CUBE_SIZE; y++)
  for(z = 0; z < CUBE_SIZE; z++) {
    
    int index = x + (CUBE_SIZE * y) + (CUBE_SIZE * CUBE_SIZE * z);
    
    positions[index] = v3(x, y, z);
    colors[index] = v3_div(v3(x, y, z), CUBE_SIZE);
    colors[index] = color_curves_map(current_curves, colors[index]);
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE, positions, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, cube_colors_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * CUBE_SIZE * CUBE_SIZE * CUBE_SIZE, colors, GL_STATIC_DRAW);
  
  free(positions);
  free(colors);

}

static void load_new_curves_file() {

  //char* filename = SDL_OpenFileDialog("Photoshop Curves (.acv)\0*.acv\0\0", 0);
  char* filename = "";
  
  if (filename[0] != '\0') {
  
    char file[MAX_PATH]; SDL_PathFileName(file, filename);
    char location[MAX_PATH]; SDL_PathFileLocation(location, filename);
    char lut_name[MAX_PATH]; strcpy(lut_name, location); strcat(lut_name, file); strcat(lut_name, ".lut");
    
    ui_text* curves_text = ui_elem_get("curves_text");
    ui_text_update_string(curves_text, file);
    
    current_curves = asset_load_get(filename);
    color_curves_write_lut(asset_get_as(filename, color_curves), lut_name);
    current_lut = asset_load_get(lut_name);
    
    lut_gen_rebuild_cube();
  }

}

static bool load_button_pressed = false;
static void load_new_curves(ui_rectangle* rect, SDL_Event event) {
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      load_button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (load_button_pressed) {
      load_button_pressed = false;
      rect->color = v4_black();
      
      load_new_curves_file();
      
    }
  }
}

void lut_gen_init() {

  asset_manager_handler("acv", acv_load_file, color_curves_delete);
  
  load_folder("./screenshots/");
  load_folder("./shaders/");
  
  camera* cam = entity_new("camera", camera);
  cam->position = v3(CUBE_SIZE * 1.5, CUBE_SIZE * 1.5, CUBE_SIZE * 1.5);
  cam->target = v3(CUBE_SIZE/2, CUBE_SIZE/2, CUBE_SIZE/2);
  
  glGenBuffers(1, &cube_positions_buffer);
  glGenBuffers(1, &cube_colors_buffer);
  
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClearDepth(1.0);
  
  ui_rectangle* top_left_rect = ui_elem_new("top_left_rect", ui_rectangle);
  top_left_rect->top_left = v2(120,10);
  top_left_rect->bottom_right = v2(390,290);
  top_left_rect->color = v4(0,0,0,1);
  top_left_rect->border_color = v4_white();
  top_left_rect->border_size = 1;
  
  ui_rectangle* top_right_rect = ui_elem_new("top_right_rect", ui_rectangle);
  top_right_rect->top_left = v2(410,10);
  top_right_rect->bottom_right = v2(790,290);
  top_right_rect->color = v4(0,0,0,1);
  top_right_rect->border_color = v4_white();
  top_right_rect->border_size = 1;
  
  ui_rectangle* bot_right_rect = ui_elem_new("bot_right_rect", ui_rectangle);
  bot_right_rect->top_left = v2(410,310);
  bot_right_rect->bottom_right = v2(790,590);
  bot_right_rect->color = v4(0,0,0,1);
  bot_right_rect->border_color = v4_white();
  bot_right_rect->border_size = 1; 
  
  ui_rectangle* curves_rect = ui_elem_new("curves_rect", ui_rectangle);
  curves_rect->top_left = v2(10, 10);
  curves_rect->bottom_right = v2(100, 35);
  curves_rect->color = v4_black();
  curves_rect->border_color = v4_white();
  curves_rect->border_size = 1;
  
  ui_text* curves_text = ui_elem_new("curves_text", ui_text);
  curves_text->position = v2(15, 15);
  curves_text->color = v4_white();
  ui_text_update_string(curves_text, "emerald");
  
  ui_rectangle* load_rect = ui_elem_new("load_rect", ui_rectangle);
  load_rect->top_left = v2(10, 45);
  load_rect->bottom_right = v2(60, 70);
  load_rect->color = v4_black();
  load_rect->border_color = v4_white();
  load_rect->border_size = 1;
  
  ui_text* load_text = ui_elem_new("load_text", ui_text);
  load_text->position = v2(15, 50);
  load_text->color = v4_white();
  ui_text_update_string(load_text, "Load");
  
  ui_elem_add_event("load_rect", load_new_curves);


  current_curves = asset_load_get("./data/emerald.acv");
  color_curves_write_lut(current_curves, "./data/emerald.lut");
  current_lut = asset_load_get("./data/emerald.lut");
  
  lut_gen_rebuild_cube();
  
}

void lut_gen_finish() {
  
  glDeleteBuffers(1, &cube_positions_buffer);
  glDeleteBuffers(1, &cube_colors_buffer);
  
}


static float proj_matrix[16];
static float view_matrix[16];

static void lut_gen_render_preview() {
  
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
  glBindTexture(GL_TEXTURE_3D, *current_lut);
  glEnable(GL_TEXTURE_3D);
  
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(viewport_width() - 390, 11, 0);
    glTexCoord2f(1, 0); glVertex3f(viewport_width() - 11, 11, 0);
    glTexCoord2f(1, 1); glVertex3f(viewport_width() - 11, 290, 0);
    glTexCoord2f(0, 1); glVertex3f(viewport_width() - 390, 290, 0);
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

static void lut_gen_render_cube() {

  glViewport(410, 10, 380, 280);
  
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
  
}

static void lut_gen_render_spline(spline* s, vector3 color) {

  spline_update(s);
  glColor3d(color.r, color.g, color.b);
  
  glEnable(GL_POINT_SMOOTH);
  glPointSize(5.0);
  glBegin(GL_POINTS);
    for(int i = 0; i < s->num_points; i++) {
      vector2 loc = v2( s->x[i], s->y[i] );
      glVertex2f(130 + (1-loc.x) * 250, 25 + loc.y * 250);
    }
  glEnd();
  glPointSize(1.0);
  glDisable(GL_POINT_SMOOTH);
  
  glEnable(GL_LINE_SMOOTH);
  glBegin(GL_LINE_STRIP);
    float step = 1.0 / 100;
    for(float j = 0; j <= 1; j += step) {
      float loc = spline_get_y(s, j);
      glVertex2f(130 + (1-j) * 250, 25 + loc * 250);
    }
    float y = s->y[s->num_points-1];
    float x = s->x[s->num_points-1];
    glVertex2f(130 + (1-x) * 250, 25 + y * 250);
  glEnd();
  glDisable(GL_LINE_SMOOTH);
  
  glColor3f(1.0, 1.0, 1.0);

}

static void lut_gen_render_curves() {
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  lut_gen_render_spline(current_curves->r_spline, v3_red());
  lut_gen_render_spline(current_curves->g_spline, v3_green());
  lut_gen_render_spline(current_curves->b_spline, v3_blue());
  lut_gen_render_spline(current_curves->rgb_spline, v3_white());
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void lut_gen_render() {
  
  lut_gen_render_cube();
  lut_gen_render_preview();
  lut_gen_render_curves();
  
}

void lut_gen_update() {
  camera* cam = entity_get("camera");

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
  
    float a1 = -(float)mouse_x * 0.005;
    float a2 = (float)mouse_y * 0.005;
    
    cam->position = v3_sub(cam->position, cam->target);
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    cam->position = v3_add(cam->position, cam->target);
    
    cam->position = v3_sub(cam->position, cam->target);
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
    cam->position = v3_add(cam->position, cam->target);
  }
  
  mouse_x = 0;
  mouse_y = 0;
}

/*
int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
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
        if ((event.motion.x > 410) && (event.motion.y > 310) &&
            (event.motion.x < 790) && (event.motion.y < 590)) {
          mouse_x = event.motion.xrel;
          mouse_y = event.motion.yrel;
        }
        break;
      }
      
      ui_event(event);
      
    }
    
    lut_gen_update();
    ui_update();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ui_render();
    lut_gen_render();
    
    SDL_GL_SwapBuffers(); 
    
    frame_end();
  }  
  
  corange_finish();
  
  return 0;
}
*/