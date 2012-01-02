#include "character.h"

character* character_new() {
  character* c = malloc(sizeof(character));
  c->position = v2_zero();
  c->velocity = v2_zero();
  return c;
}

void character_delete(character* c) {
  free(c);
}

void character_update(character* c) {
  
  float timestep = frame_time();
  
  c->velocity = v2_clamp(c->velocity, -0.5, 0.5);
  //c->velocity = v2_add(c->velocity, v2(0, -0.001) );
  
  //c->position = v2_add(c->position, v2_div(c->velocity, timestep));
  c->position = v2_add(c->position, c->velocity);
}

void character_render(character* c, vector2 camera_position) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(camera_position.x - viewport_width() / 2, 
          camera_position.x + viewport_width() / 2,
          camera_position.y + viewport_height() / 2,
          camera_position.y - viewport_height() / 2
          , -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_TEXTURE_2D);
  
  texture* character_tex = asset_get("./tiles/character.dds");
  glBindTexture(GL_TEXTURE_2D, *character_tex);
  
  glBegin(GL_QUADS);
    
    glVertex3f(c->position.x, c->position.y + 32, 0);
    glTexCoord2f(0, 1);
    glVertex3f(c->position.x + 32, c->position.y + 32, 0);
    glTexCoord2f(1, 1);
    glVertex3f(c->position.x + 32, c->position.y, 0);
    glTexCoord2f(1, 0);
    glVertex3f(c->position.x, c->position.y, 0);
    glTexCoord2f(0, 0);
    
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  glEnable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}