#include "ui_button.h"

#include "error.h"
#include "viewport.h"

ui_button* ui_button_new() {

  ui_button* b = malloc(sizeof(ui_button));
  
  b->top_left = v2(10, 10);
  b->bottom_right = v2(75, 35);
  
  b->label = ui_text_new_string("Button1");
  b->label->position = v2(15, 15);
  b->label->color = v4_white();
  ui_text_update_properties(b->label);
  
  b->color = v4_black();
  b->pressed_color = v4_grey();
  
  b->texture = NULL;

  b->border_size = 1;
  b->border_color = v4_white();
  
  b->active = true;
  b->enabled = true;
  b->pressed = false;
  
  return b;

}

void ui_button_delete(ui_button* b) {
  
  ui_text_delete(b->label);
  free(b);
  
}

void ui_button_update(ui_button* b) {
  
  ui_text_update(b->label);
  
}

void ui_button_move(ui_button* b, vector2 pos) {
  
  vector2 offset = v2_sub(b->bottom_right, b->top_left);
  vector2 label_offset = v2_sub(b->label->position, b->top_left);
  
  b->top_left = pos;
  b->bottom_right = v2_add(pos, offset);
  b->label->position = v2_add(pos, label_offset);
  
  ui_text_update_properties(b->label);
}

void ui_button_resize(ui_button* b, vector2 size) {
  
  b->bottom_right = v2_add(b->top_left, size);
  
}

void ui_button_set_label(ui_button* b, char* label) {
  
  ui_text_update_string(b->label, label);
  
}

void ui_button_render(ui_button* b) {
  
  if (!b->active) {
    return;
  }
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  if(b->texture != NULL) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, *(b->texture) );
  } else {
    glActiveTexture(GL_TEXTURE0 + 0);
    glDisable(GL_TEXTURE_2D);
  }
  
  if (b->pressed || !b->enabled) {
    glColor4f(b->pressed_color.r, b->pressed_color.g, b->pressed_color.b, b->pressed_color.a);
  } else {
    glColor4f(b->color.r, b->color.g, b->color.b, b->color.a);
  }
  
  glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(b->top_left.x, b->top_left.y, 0);
    glTexCoord2f(1, 1); glVertex3f(b->bottom_right.x, b->top_left.y, 0);
    glTexCoord2f(1, 0); glVertex3f(b->bottom_right.x, b->bottom_right.y, 0);
    glTexCoord2f(0, 0); glVertex3f(b->top_left.x, b->bottom_right.y, 0);
  glEnd();
  
  if(b->border_size > 0) {
  
    glColor4f(b->border_color.r, b->border_color.g, b->border_color.b, b->border_color.a);  
    glLineWidth(b->border_size);
    
    glBegin(GL_LINE_STRIP);
      
      glVertex3f(b->top_left.x, b->top_left.y, 0);
      glVertex3f(b->bottom_right.x, b->top_left.y, 0);
      glVertex3f(b->bottom_right.x, b->bottom_right.y, 0);
      glVertex3f(b->top_left.x, b->bottom_right.y, 0);
      glVertex3f(b->top_left.x, b->top_left.y, 0);
      
    glEnd();
  
    glLineWidth(1);
  }
  
  glColor4f(1, 1, 1, 1);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  if(b->texture != NULL) {
    glDisable(GL_TEXTURE_2D);
  }
  
  ui_text_render(b->label);
  
}

void ui_button_set_texture(ui_button* b, texture* t) {
  b->texture = t;
}

void ui_button_disable(ui_button* b) {
  b->enabled = false;
}

void ui_button_enable(ui_button* b) {
  b->enabled = true;
}

bool ui_button_contains_position(ui_button* b, vector2 pos) {

  if (!b->active) { return false; }
  
  if(( pos.x > b->top_left.x ) && ( pos.x < b->bottom_right.x ) &&
     ( pos.y > b->top_left.y ) && ( pos.y < b->bottom_right.y )) {
    return true;
  } else {
    return false;
  }

}
