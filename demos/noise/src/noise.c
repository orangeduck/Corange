#include "corange.h"

#include "perlin_noise.h"

#include "noise.h"

static float time = 0.0;

void noise_render() {
  
  glClear(GL_COLOR_BUFFER_BIT);
  
  glDisable(GL_LIGHTING);
  
  glUseProgram(*asset_get_as("./shaders/noise.prog",shader));
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *asset_get_as("$CORANGE/resources/random.dds", texture));
  glUniform1i(glGetUniformLocation(*asset_get_as("./shaders/noise.prog",shader), "noise_texture"), 0);
  
  time += frame_time();
  glUniform1f(glGetUniformLocation(*asset_get_as("./shaders/noise.prog",shader), "time"), time);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
	
	glActiveTexture(GL_TEXTURE0 + 0);
  glDisable(GL_TEXTURE_2D);
	
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
  glEnable(GL_LIGHTING);
  
  SDL_GL_SwapBuffers(); 

}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  viewport_set_dimensions( v2(512, 512) );
  
  load_folder("./shaders/");
  load_file("$CORANGE/resources/random.dds");
  
  glClearColor(1.0, 0.0, 0.0, 1.0);
  
  bool running = true;
  while(running) {
    frame_begin();
    
    SDL_Event event;
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
    }
    
    noise_render();
    
    frame_end();
  }  
  
  corange_finish();
  
  return 0;
}
