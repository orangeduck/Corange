#include "character.h"

character* character_new() {
  character* c = malloc(sizeof(character));
  c->position = v2_zero();
  c->velocity = v2_zero();
  c->flap_timer = 0.0;
  c->facing_left = false;
  return c;
}

void character_delete(character* c) {
  free(c);
}

void character_update(character* c) {
  c->velocity.x = clamp(c->velocity.x, -7.0, 7.0);
  c->position = v2_add(c->position, c->velocity);
  
  if (c->flap_timer > 0.0) {
    c->flap_timer -= frame_time();
  }
}

static float previous_x = 0.0;


/* Renders a simple quad to the screen */

void character_render(character* c, vector2 camera_position) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(camera_position.x - graphics_viewport_width() / 2, 
          camera_position.x + graphics_viewport_width() / 2,
          -camera_position.y + graphics_viewport_height() / 2,
          -camera_position.y - graphics_viewport_height() / 2
          , -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_TEXTURE_2D);
  
  /* Conditional as to if we render flap or normal icon */
  texture* character_tex;
  if (c->flap_timer > 0.0) {
    character_tex = asset_get("./tiles/character_flap.dds");
  } else {
    character_tex = asset_get("./tiles/character.dds");
  }
  glBindTexture(GL_TEXTURE_2D, *character_tex);
  
  /* Swaps the direction of the uvs when facing the opposite direction */
  if (c->facing_left) {
  
    glBegin(GL_TRIANGLES);
      glTexCoord2f(1, 1); glVertex3f(c->position.x, c->position.y + 32, 0);
      glTexCoord2f(1, 0); glVertex3f(c->position.x, c->position.y, 0);
      glTexCoord2f(0, 0); glVertex3f(c->position.x + 32, c->position.y, 0);
      
      glTexCoord2f(1, 1); glVertex3f(c->position.x, c->position.y + 32, 0);
      glTexCoord2f(0, 1); glVertex3f(c->position.x + 32, c->position.y + 32, 0);
      glTexCoord2f(0, 0);glVertex3f(c->position.x + 32, c->position.y, 0);
    glEnd();
    
  } else {
  
    glBegin(GL_TRIANGLES);
      glTexCoord2f(0, 1); glVertex3f(c->position.x, c->position.y + 32, 0);
      glTexCoord2f(0, 0); glVertex3f(c->position.x, c->position.y, 0);
      glTexCoord2f(1, 0); glVertex3f(c->position.x + 32, c->position.y, 0);
      
      glTexCoord2f(0, 1); glVertex3f(c->position.x, c->position.y + 32, 0);
      glTexCoord2f(1, 1); glVertex3f(c->position.x + 32, c->position.y + 32, 0);
      glTexCoord2f(1, 0);glVertex3f(c->position.x + 32, c->position.y, 0);
    glEnd();
    
  }
  
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}