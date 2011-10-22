#include "corange.h"

#include "perlin_noise.h"

texture* perlin_noise;

void noise_init() {
  
  viewport_set_multisamples(8);
  viewport_set_width(512);
  viewport_set_height(512);
  
  perlin_noise = perlin_noise_generate_texture2D(512, 512, 8);
  
  texture_write_to_file(perlin_noise, "./perlin_noise.tga");
  
}

void noise_update() {

}

void noise_render() {
  
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *perlin_noise);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
}

void noise_event(SDL_Event event) {

}

void noise_finish() {

  texture_delete(perlin_noise);

}