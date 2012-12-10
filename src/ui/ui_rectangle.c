#include "ui/ui_rectangle.h"

#include "cgraphics.h"

#include "assets/texture.h"
#include "assets/shader.h"
#include "assets/material.h"

ui_rectangle* ui_rectangle_new() {

  ui_rectangle* rect = malloc(sizeof(ui_rectangle));
  
  rect->top_left = vec2_new(10, 10);
  rect->bottom_right = vec2_new(20, 20);
  rect->color = vec4_white();
  rect->glitch = 0.0;
  
  rect->texture = asset_hndl_null();
  rect->texture_width = 1;
  rect->texture_height = 1;
  rect->texture_tile = false;

  rect->border_size = 0.0;
  rect->border_color = vec4_black();
  
  rect->active = true;
  
  return rect;
  
}

void ui_rectangle_delete(ui_rectangle* rect) {
  free(rect);
}

void ui_rectangle_event(ui_rectangle* rect, SDL_Event e) {

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

void ui_rectangle_set_texture(ui_rectangle* rect, asset_hndl tex, int width, int height, bool tile) {
  rect->texture = tex;
  rect->texture_width = width;
  rect->texture_height = height;
  rect->texture_tile = tile;
}

void ui_rectangle_set_border(ui_rectangle* rect, float size, vec4 color) {
  rect->border_size = size;
  rect->border_color = color;
}

void ui_rectangle_set_color(ui_rectangle* rect, vec4 color) {
  rect->color = color;
}

vec2 ui_rectangle_center(ui_rectangle* rect) {
  return vec2_div(vec2_add(rect->top_left, rect->bottom_right), 2);
}

void ui_rectangle_set_glitch(ui_rectangle* rect, float glitch) {
  rect->glitch = glitch;
}

static float TIME = 0;

void ui_rectangle_render(ui_rectangle* rect) {
  
  if(!rect->active) {
    return;
  }
  
  asset_hndl mat = asset_hndl_new_load(P("$CORANGE/shaders/ui.mat"));
  
  shader_program* program_ui = material_get_entry(asset_hndl_ptr(mat), 0)->program;
  
  GLuint ui_handle = shader_program_handle(program_ui);
  glUseProgram(ui_handle);
  
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
    glUniform1i(glGetUniformLocation(ui_handle, "diffuse"), 0);
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(asset_hndl_new_load(P("$CORANGE/resources/random.dds")))) );
    glUniform1i(glGetUniformLocation(ui_handle, "random"), 1);
  }
  
  TIME += frame_time();
  glUniform1f(glGetUniformLocation(ui_handle, "time"), TIME);
  glUniform1f(glGetUniformLocation(ui_handle, "glitch"), rect->glitch);
  
  glColor4f(rect->color.x, rect->color.y, rect->color.z, rect->color.w);  
  
  if (rect->texture_tile) {
  
  float width = rect->bottom_right.x - rect->top_left.x;
  float height = rect->bottom_right.y - rect->top_left.y;
  
    glBegin(GL_QUADS);
      glTexCoord2f(0, height / rect->texture_height);
      glVertex3f(rect->top_left.x, rect->top_left.y, 0);
      
      glTexCoord2f(width / rect->texture_width, height / rect->texture_height);
      glVertex3f(rect->bottom_right.x, rect->top_left.y, 0);
      
      glTexCoord2f(width / rect->texture_width, 0);
      glVertex3f(rect->bottom_right.x, rect->bottom_right.y, 0);
      
      glTexCoord2f(0, 0);
      glVertex3f(rect->top_left.x, rect->bottom_right.y, 0);
    glEnd();
  
  } else {
  
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1); glVertex3f(rect->top_left.x, rect->top_left.y, 0);
      glTexCoord2f(1, 1); glVertex3f(rect->bottom_right.x, rect->top_left.y, 0);
      glTexCoord2f(1, 0); glVertex3f(rect->bottom_right.x, rect->bottom_right.y, 0);
      glTexCoord2f(0, 0); glVertex3f(rect->top_left.x, rect->bottom_right.y, 0);
    glEnd();
  
  }
  
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
    glActiveTexture(GL_TEXTURE0 + 1);
    glDisable(GL_TEXTURE_2D);
  
    glActiveTexture(GL_TEXTURE0 + 0);
    glDisable(GL_TEXTURE_2D);
  }
  
  glUseProgram(0);
 
}

bool ui_rectangle_contains_point(ui_rectangle* rect, vec2 pos) {

  if (!rect->active) { return false; }

  if(( pos.x > rect->top_left.x ) && ( pos.x < rect->bottom_right.x ) &&
     ( pos.y > rect->top_left.y ) && ( pos.y < rect->bottom_right.y )) {
    return true;
  } else {
    return false;
  }
}
