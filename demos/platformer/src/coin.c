#include <stdlib.h>

#include "coin.h"

coin* coin_new() {
  coin* c = malloc(sizeof(coin));
  c->position = vec2_zero();
  return c;
}

void coin_delete(coin* c) {
  free(c);
}

/* Renders a quad to the screen with coin texture */

void coin_render(coin* c, vec2 camera_position) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(camera_position.x - graphics_viewport_width() / 2, 
          camera_position.x + graphics_viewport_width() / 2,
          -camera_position.y + graphics_viewport_height() / 2,
          -camera_position.y - graphics_viewport_height() / 2
          , 0, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_TEXTURE_2D);
  
  texture* coin_tex = asset_get(P("./tiles/coin.dds"));
  glBindTexture(GL_TEXTURE_2D, texture_handle(coin_tex));
  
  glBegin(GL_QUADS);
    
    glTexCoord2f(0, 1); glVertex3f(c->position.x, c->position.y + 32, 0);
    glTexCoord2f(1, 1); glVertex3f(c->position.x + 32, c->position.y + 32, 0);
    glTexCoord2f(1, 0); glVertex3f(c->position.x + 32, c->position.y, 0);
    glTexCoord2f(0, 0); glVertex3f(c->position.x, c->position.y, 0);
    
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}