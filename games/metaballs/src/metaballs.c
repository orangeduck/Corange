#include "corange.h"

#include "particles.h"

void metaballs_init() {
  
  load_folder("/kernels/");
  
  particles_init();
  
  glClearColor(0.5, 0.5, 0.5, 1.0);
  glPointSize(5.0);
  glEnable(GL_POINT_SMOOTH);
  
}

void metaballs_update() {

  particles_update(0.0001);
  
}

void metaballs_render() {

  glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.1, 0.1, 0, 0.1, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
  int i;
  for(i = 0; i < particles_count(); i++) {
    
    vector3 position = particle_position(i);
    
    glBegin(GL_POINTS);
    glVertex3f(position.x, position.y, position.z);
    glEnd();
    
  }
  
}

void metaballs_event(SDL_Event event) {

}

void metaballs_finish() {
  
  particles_finish();

}