#include "viewport.h"
#include "timing.h"
#include "matrix.h"
#include "asset_manager.h"

#include "ui_spinner.h"

ui_spinner* ui_spinner_new() {
  ui_spinner* s = malloc(sizeof(ui_spinner));
  s->top_left = v2_zero();
  s->bottom_right = v2(32, 32);
  s->color = v4_black();
  s->texture = asset_load_get("$CORANGE/ui/spinner.dds");
  s->speed = 5;
  s->rotation = 0;
  s->active = true;
  return s;
}

void ui_spinner_delete(ui_spinner* s) {
  free(s);
}

void ui_spinner_update(ui_spinner* s) {
  s->rotation += s->speed * frame_time();
}


void ui_spinner_render(ui_spinner* s) {

  vector2 top_left = s->top_left;
  vector2 top_right = v2(s->bottom_right.x, s->top_left.y);
  vector2 bot_left = v2(s->top_left.x, s->bottom_right.y);
  vector2 bot_right = s->bottom_right;
  
  vector2 center;
  center.x = (top_left.x + top_right.x) / 2;
  center.y = (top_left.y + bot_left.y) / 2;
  
  top_left = v2_sub(top_left, center);
  top_right = v2_sub(top_right, center);
  bot_left = v2_sub(bot_left, center);
  bot_right = v2_sub(bot_right, center);
  
  matrix_2x2 rot = m22_rotation(s->rotation);
  
  top_left = m22_mul_v2(rot, top_left);
  top_right = m22_mul_v2(rot, top_right);
  bot_left = m22_mul_v2(rot, bot_left);
  bot_right = m22_mul_v2(rot, bot_right);
  
  top_left = v2_add(top_left, center);
  top_right = v2_add(top_right, center);
  bot_left = v2_add(bot_left, center);
  bot_right = v2_add(bot_right, center);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *(s->texture) );
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor4f(s->color.r, s->color.g, s->color.b, s->color.a);
  
  glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(top_left.x, top_left.y, 0);
    glTexCoord2f(1, 1); glVertex3f(bot_left.x, bot_left.y, 0);
    glTexCoord2f(1, 0); glVertex3f(bot_right.x, bot_right.y, 0);
    glTexCoord2f(0, 0); glVertex3f(top_right.x, top_right.y, 0);
  glEnd();
  
  glColor4f(1, 1, 1, 1);
  
  glDisable(GL_BLEND);
  
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}