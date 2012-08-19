#include "ui/ui_button.h"

#include "graphics_manager.h"

#include "assets/texture.h"

ui_button* ui_button_new() {

  ui_button* b = malloc(sizeof(ui_button));
  
  b->top_left = vec2_new(10, 10);
  b->bottom_right = vec2_new(75, 35);
  
  b->label = ui_text_new_string("Button1");
  b->label->position = vec2_new(15, 15);
  b->label->color = vec4_white();
  ui_text_draw(b->label);
  
  b->color = vec4_black();
  b->pressed_color = vec4_grey();
  
  b->texture = asset_hndl_null();

  b->border_size = 1;
  b->border_color = vec4_white();
  
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

void ui_button_move(ui_button* b, vec2 pos) {
  
  vec2 offset = vec2_sub(b->bottom_right, b->top_left);
  vec2 label_offset = vec2_sub(b->label->position, b->top_left);
  
  b->top_left = pos;
  b->bottom_right = vec2_add(pos, offset);
  b->label->position = vec2_add(pos, label_offset);
  
  ui_text_draw(b->label);
}

void ui_button_resize(ui_button* b, vec2 size) {
  
  b->bottom_right = vec2_add(b->top_left, size);
  
}

void ui_button_set_label(ui_button* b, char* label) {
  
  ui_text_draw_string(b->label, label);
  
}

void ui_button_render(ui_button* b) {
  
  if (!b->active) {
    return;
  }
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, graphics_viewport_width(), graphics_viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  if(!asset_hndl_isnull(b->texture)) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(b->texture)) );
  } else {
    glActiveTexture(GL_TEXTURE0 + 0);
    glDisable(GL_TEXTURE_2D);
  }
  
  if (b->pressed || !b->enabled) {
    glColor4f(b->pressed_color.x, b->pressed_color.y, b->pressed_color.z, b->pressed_color.w);
  } else {
    glColor4f(b->color.x, b->color.y, b->color.z, b->color.w);
  }
  
  glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(b->top_left.x, b->top_left.y, 0);
    glTexCoord2f(1, 1); glVertex3f(b->bottom_right.x, b->top_left.y, 0);
    glTexCoord2f(1, 0); glVertex3f(b->bottom_right.x, b->bottom_right.y, 0);
    glTexCoord2f(0, 0); glVertex3f(b->top_left.x, b->bottom_right.y, 0);
  glEnd();
  
  if(b->border_size > 0) {
  
    glColor4f(b->border_color.x, b->border_color.y, b->border_color.z, b->border_color.w);  
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
  
  if(!asset_hndl_isnull(b->texture)) {
    glDisable(GL_TEXTURE_2D);
  }
  
  ui_text_render(b->label);
  
}

void ui_button_disable(ui_button* b) {
  b->enabled = false;
}

void ui_button_enable(ui_button* b) {
  b->enabled = true;
}

bool ui_button_contains_position(ui_button* b, vec2 pos) {

  if (!b->active) { return false; }
  
  if(( pos.x > b->top_left.x ) && ( pos.x < b->bottom_right.x ) &&
     ( pos.y > b->top_left.y ) && ( pos.y < b->bottom_right.y )) {
    return true;
  } else {
    return false;
  }

}
