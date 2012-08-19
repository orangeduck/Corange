#include "ui/ui_rectangle.h"

#include "cgraphics.h"

#include "assets/texture.h"

ui_rectangle* ui_rectangle_new() {

  ui_rectangle* rect = malloc(sizeof(ui_rectangle));
  
  rect->top_left = vec2_new(10, 10);
  rect->bottom_right = vec2_new(20, 20);
  rect->color = vec4_white();
  
  rect->texture = asset_hndl_null();

  rect->border_size = 0.0;
  rect->border_color = vec4_black();
  
  rect->active = true;
  
  return rect;
  
}

void ui_rectangle_delete(ui_rectangle* rect) {
  free(rect);
}

void ui_rectangle_update(ui_rectangle* rect) {
  
}

void ui_rectangle_move(ui_rectangle* rect, vec2 pos) {
  vec2 size = vec2_sub(rect->bottom_right, rect->top_left);
  rect->top_left = pos;
  rect->bottom_right = vec2_add(pos, size);
}

void ui_rectangle_resize(ui_rectangle* rect, vec2 size) {
  rect->bottom_right = vec2_add(rect->top_left, size);
}

void ui_rectangle_render(ui_rectangle* rect) {
  
  if(!rect->active) {
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
  
  if(!asset_hndl_isnull(rect->texture)) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(rect->texture)) );
  }
  
  glColor4f(rect->color.x, rect->color.y, rect->color.z, rect->color.w);  
    
  glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(rect->top_left.x, rect->top_left.y, 0);
    glTexCoord2f(1, 1); glVertex3f(rect->bottom_right.x, rect->top_left.y, 0);
    glTexCoord2f(1, 0); glVertex3f(rect->bottom_right.x, rect->bottom_right.y, 0);
    glTexCoord2f(0, 0); glVertex3f(rect->top_left.x, rect->bottom_right.y, 0);
  glEnd();
  
  if(rect->border_size > 0) {
  
    glColor4f(rect->border_color.x, rect->border_color.y, rect->border_color.z, rect->border_color.w);  
    glLineWidth(rect->border_size);
    
    glBegin(GL_LINE_STRIP);
      
      glVertex3f(rect->top_left.x, rect->top_left.y, 0);
      glVertex3f(rect->bottom_right.x, rect->top_left.y, 0);
      glVertex3f(rect->bottom_right.x, rect->bottom_right.y, 0);
      glVertex3f(rect->top_left.x, rect->bottom_right.y, 0);
      glVertex3f(rect->top_left.x, rect->top_left.y, 0);
      
    glEnd();
  
    glLineWidth(1);
  }
  
  glColor4f(1, 1, 1, 1);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  if(!asset_hndl_isnull(rect->texture)) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glDisable(GL_TEXTURE_2D);
  }
 
}

bool ui_rectangle_contains_position(ui_rectangle* rect, vec2 pos) {

  if (!rect->active) { return false; }

  if(( pos.x > rect->top_left.x ) && ( pos.x < rect->bottom_right.x ) &&
     ( pos.y > rect->top_left.y ) && ( pos.y < rect->bottom_right.y )) {
    return true;
  } else {
    return false;
  }
}
