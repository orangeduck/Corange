#include <time.h>

#include "corange.h"

#include "perlin_noise.h"

#include "noise.h"

static float shader_time = 0.0;

void noise_render() {
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  material* noise_mat = asset_get(P("./shaders/noise.mat"));
  
  GLuint handle = shader_program_handle(material_get_entry(noise_mat, 0)->program);
  GLuint random_tex = texture_handle(asset_get(P("$CORANGE/resources/random.dds")));
  
  glUseProgram(handle);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, random_tex);
  glUniform1i(glGetUniformLocation(handle, "noise_texture"), 0);
  
  glUniform1f(glGetUniformLocation(handle, "time"), shader_time);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
	
	glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
	
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
}

static bool currently_saving = false;
static int save_noise_to_file_thread(void* unused) {

  image* noise = perlin_noise_generate(512, 512, 8);
  tga_save_file(noise, "./perlin_noise.tga");
  debug("Noise saved as perlin_noise.tga");
  image_delete(noise);
  
  ui_spinner* save_spinner = ui_elem_get("save_spinner");
  ui_rectangle* spinner_box = ui_elem_get("spinner_box");
  save_spinner->color = vec4_new(1,1,1,0);
  spinner_box->color = vec4_new(0,0,0,0);
  spinner_box->border_color = vec4_new(1,1,1,0);
  
  currently_saving = false;
  
  return 0;
}

static SDL_Thread* save_thread = NULL;
static void save_noise_to_file(ui_button* b, SDL_Event event) {
  
  if (currently_saving) {
    return;
  }
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_button_contains_position(b, vec2_new(event.motion.x, event.motion.y))) {
      b->pressed = true;
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (b->pressed) {
      b->pressed = false;
      currently_saving = true;
      
      ui_spinner* save_spinner = ui_elem_get("save_spinner");
      save_spinner->color = vec4_new(1,1,1,1);
      
      save_thread = SDL_CreateThread(save_noise_to_file_thread, NULL);
      
    }
  }
  
}


int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Noise");
  
  folder_load(P("./shaders/"));
  file_load(P("$CORANGE/resources/random.dds"));
  
  glClearColor(1.0, 0.0, 0.0, 1.0);
  
  ui_button* info_button = ui_elem_new("info_button", ui_button);
  ui_button_move(info_button, vec2_new(10, 10));
  ui_button_resize(info_button, vec2_new(460,25));
  ui_button_set_label(info_button, "Procedural texture from perlin noise and feedback functions.");
  
  ui_button* save_button = ui_elem_new("save_button", ui_button);
  save_button->bottom_right = vec2_new(860, 35);
  ui_button_move(save_button, vec2_new(480, 10));
  ui_button_resize(save_button, vec2_new(380,25));
  ui_button_set_label(save_button, "Click Here to save tileable perlin noise to file.");
  ui_elem_add_event("save_button", save_noise_to_file);
  
  ui_rectangle* spinner_box = ui_elem_new("spinner_box", ui_rectangle);
  spinner_box->color = vec4_black();
  spinner_box->border_color = vec4_white();
  spinner_box->border_size = 1;
  spinner_box->top_left = vec2_new(870, 7);
  spinner_box->bottom_right = vec2_new(902, 39);
  
  ui_spinner* save_spinner = ui_elem_new("save_spinner", ui_spinner);
  save_spinner->color = vec4_new(1,1,1,0);
  save_spinner->top_left = vec2_new(874, 11);
  save_spinner->bottom_right = vec2_add(save_spinner->top_left, vec2_new(24,24));
  
  srand(time(NULL));
  shader_time = (float)rand() / (RAND_MAX / 1000);
  
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
      
      ui_event(event);
      
    }
    
    shader_time += frame_time();
    ui_update();
    
    noise_render();
    ui_render();
    
    SDL_GL_SwapBuffers();
    
    frame_end();
    
  }  
  
  SDL_WaitThread(save_thread, NULL);
  
  corange_finish();
  
  return 0;
}
