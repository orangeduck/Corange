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
  
  glUseProgram(*asset_get_as("./shaders/noise.prog",shader));
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *asset_get_as("$CORANGE/resources/random.dds", texture));
  glUniform1i(glGetUniformLocation(*asset_get_as("./shaders/noise.prog",shader), "noise_texture"), 0);
  
  glUniform1f(glGetUniformLocation(*asset_get_as("./shaders/noise.prog",shader), "time"), shader_time);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
	
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
  save_spinner->color = v4(1,1,1,0);
  spinner_box->color = v4(0,0,0,0);
  spinner_box->border_color = v4(1,1,1,0);
  
  currently_saving = false;
  
  return 0;
}

static SDL_Thread* save_thread = NULL;
static bool button_pressed = false;
static void save_noise_to_file(ui_rectangle* rect, SDL_Event event) {
  
  if (currently_saving) {
    return;
  }
  
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      button_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (button_pressed) {
      button_pressed = false;
      currently_saving = true;
      
      rect->color = v4_black();
      
      ui_spinner* save_spinner = ui_elem_get("save_spinner");
      ui_rectangle* spinner_box = ui_elem_get("spinner_box");
      save_spinner->color = v4(1,1,1,1);
      spinner_box->color = v4_black();
      spinner_box->border_color = v4_white();
      
      save_thread = SDL_CreateThread(save_noise_to_file_thread, NULL);
      
    }
  }
  
}


int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Noise");
  
  load_folder("./shaders/");
  load_file("$CORANGE/resources/random.dds");
  
  glClearColor(1.0, 0.0, 0.0, 1.0);
  
  ui_rectangle* info_rect = ui_elem_new("info_rect", ui_rectangle);
  info_rect->top_left = v2(10, 10);
  info_rect->bottom_right = v2(470, 35);
  info_rect->color = v4_black();
  info_rect->border_color = v4_white();
  info_rect->border_size = 1;
  
  ui_text* info_text = ui_elem_new("info_text", ui_text);
  info_text->position = v2(18, 15);
  info_text->color = v4_white();
  ui_text_update_string(info_text, "Procedural texture from perlin noise and feedback functions.");
  
  ui_rectangle* save_rect = ui_elem_new("save_rect", ui_rectangle);
  save_rect->top_left = v2(480, 10);
  save_rect->bottom_right = v2(860, 35);
  save_rect->color = v4_black();
  save_rect->border_color = v4_white();
  save_rect->border_size = 1;
  ui_elem_add_event("save_rect", save_noise_to_file);
  
  ui_text* save_text = ui_elem_new("save_text", ui_text);
  save_text->position = v2(488, 15);
  save_text->color = v4_white();
  ui_text_update_string(save_text, "Click Here to save tileable perlin noise to file.");
  
  ui_rectangle* spinner_box = ui_elem_new("spinner_box", ui_rectangle);
  spinner_box->top_left = v2(870, 7);
  spinner_box->bottom_right = v2(902, 39);
  spinner_box->color = v4(0,0,0,0);
  spinner_box->border_color = v4(1,1,1,0);
  spinner_box->border_size = 1;
  
  ui_spinner* save_spinner = ui_elem_new("save_spinner", ui_spinner);
  save_spinner->color = v4(1,1,1,0);
  save_spinner->top_left = v2(874, 11);
  save_spinner->bottom_right = v2_add(save_spinner->top_left, v2(24,24));
  
  srand(time(NULL));
  shader_time = rand();
  
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
