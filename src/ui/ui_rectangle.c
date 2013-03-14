#include "ui/ui_rectangle.h"

#include "cgraphics.h"

#include "assets/texture.h"
#include "assets/shader.h"
#include "assets/material.h"

ui_rectangle* ui_rectangle_new() {

  ui_rectangle* r = malloc(sizeof(ui_rectangle));
  
  r->top_left = vec2_new(10, 10);
  r->bottom_right = vec2_new(20, 20);
  r->color = vec4_white();
  r->glitch = 0.0;
  r->time = 0.0;
  
  r->texture = asset_hndl_null();
  r->texture_width = 1;
  r->texture_height = 1;
  r->texture_tile = false;

  r->border_size = 0.0;
  r->border_color = vec4_black();
  
  r->blend_src = GL_SRC_ALPHA;
  r->blend_dst = GL_ONE_MINUS_SRC_ALPHA;

  r->active = true;
  
  return r;
  
}

void ui_rectangle_delete(ui_rectangle* r) {
  free(r);
}

void ui_rectangle_event(ui_rectangle* r, SDL_Event e) {

}

void ui_rectangle_update(ui_rectangle* r) {
  r->time += frame_time();
}

void ui_rectangle_move(ui_rectangle* r, vec2 pos) {
  vec2 size = vec2_sub(r->bottom_right, r->top_left);
  r->top_left = pos;
  r->bottom_right = vec2_add(pos, size);
}

void ui_rectangle_resize(ui_rectangle* r, vec2 size) {
  r->bottom_right = vec2_add(r->top_left, size);
}

void ui_rectangle_set_texture(ui_rectangle* r, asset_hndl tex, int width, int height, bool tile) {
  r->texture = tex;
  r->texture_width = width;
  r->texture_height = height;
  r->texture_tile = tile;
}

void ui_rectangle_set_border(ui_rectangle* r, float size, vec4 color) {
  r->border_size = size;
  r->border_color = color;
}

void ui_rectangle_set_color(ui_rectangle* r, vec4 color) {
  r->color = color;
}

vec2 ui_rectangle_center(ui_rectangle* r) {
  return vec2_div(vec2_add(r->top_left, r->bottom_right), 2);
}

void ui_rectangle_set_glitch(ui_rectangle* r, float glitch) {
  r->glitch = glitch;
}

void ui_rectangle_blend(ui_rectangle* r, GLenum blend_src, GLenum blend_dst) {
  r->blend_src = blend_src;
  r->blend_dst = blend_dst;
}

void ui_rectangle_render(ui_rectangle* r) {
  
  if(!r->active) { return; }
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  asset_hndl mat = asset_hndl_new_load(P("$CORANGE/shaders/ui.mat"));  
  shader_program* program_ui = material_get_entry(asset_hndl_ptr(&mat), 0)->program;
  
  shader_program_enable(program_ui);  
  shader_program_set_mat4(program_ui, "world", mat4_id());
  shader_program_set_mat4(program_ui, "view", mat4_id());
  shader_program_set_mat4(program_ui, "proj", mat4_orthographic(0, width, height, 0, -1, 1));
  
  glEnable(GL_BLEND);
  glBlendFunc(r->blend_src, r->blend_dst);
  
  if (!asset_hndl_isnull(&r->texture)) {
    shader_program_set_texture(program_ui, "diffuse", 0, r->texture);
    shader_program_set_texture(program_ui, "random",  1, asset_hndl_new_load(P("$CORANGE/resources/random.dds")));
  }
  
  shader_program_set_float(program_ui, "time", r->time);
  shader_program_set_float(program_ui, "glitch", r->glitch);
  
  float rect_colors[] = {
    r->color.x, r->color.y, r->color.z, r->color.w,
    r->color.x, r->color.y, r->color.z, r->color.w,
    r->color.x, r->color.y, r->color.z, r->color.w,
    r->color.x, r->color.y, r->color.z, r->color.w,
    r->color.x, r->color.y, r->color.z, r->color.w,
    r->color.x, r->color.y, r->color.z, r->color.w
  };
  
  float rect_positions[] = {
    r->top_left.x,     r->top_left.y,
    r->top_left.x,     r->bottom_right.y,
    r->bottom_right.x, r->bottom_right.y,
    r->top_left.x,     r->top_left.y,
    r->bottom_right.x, r->top_left.y,
    r->bottom_right.x, r->bottom_right.y,
  };
  
  float rect_texcoords[12];
  
  if (r->texture_tile) {
  
    float width = r->bottom_right.x - r->top_left.x;
    float height = r->bottom_right.y - r->top_left.y;
    
    rect_texcoords[0]  = 0; 
    rect_texcoords[1]  = height / r->texture_height;
    rect_texcoords[2]  = width  / r->texture_width;
    rect_texcoords[3]  = height / r->texture_height;
    rect_texcoords[4]  = width  / r->texture_width;
    rect_texcoords[5]  = 0;
    rect_texcoords[6]  = 0; 
    rect_texcoords[7]  = height / r->texture_height;
    rect_texcoords[8]  = 0;
    rect_texcoords[9]  = 0;
    rect_texcoords[10] = width  / r->texture_width;
    rect_texcoords[11] = 0;
      
  } else {
  
    rect_texcoords[0]  = 0; rect_texcoords[1]  = 0; 
    rect_texcoords[2]  = 1; rect_texcoords[3]  = 0; 
    rect_texcoords[4]  = 1; rect_texcoords[5]  = 1; 
    rect_texcoords[6]  = 0; rect_texcoords[7]  = 0; 
    rect_texcoords[8]  = 0; rect_texcoords[9]  = 1; 
    rect_texcoords[10] = 1; rect_texcoords[11] = 1; 
  
  }
  
  shader_program_enable_attribute(program_ui, "vPosition", 2, 2, rect_positions);
  shader_program_enable_attribute(program_ui, "vTexcoord", 2, 2, rect_texcoords);
  shader_program_enable_attribute(program_ui, "vColor", 4, 4, rect_colors);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
  shader_program_disable_attribute(program_ui, "vPosition");
  shader_program_disable_attribute(program_ui, "vTexcoord");
  shader_program_disable_attribute(program_ui, "vColor");
  
  SDL_GL_CheckError();
  
  if (r->border_size > 0) {
    
    glLineWidth(r->border_size);
    
    float border_colors[] = {
      r->border_color.x, r->border_color.y, r->border_color.z, r->border_color.w,
      r->border_color.x, r->border_color.y, r->border_color.z, r->border_color.w,
      r->border_color.x, r->border_color.y, r->border_color.z, r->border_color.w,
      r->border_color.x, r->border_color.y, r->border_color.z, r->border_color.w,
      r->border_color.x, r->border_color.y, r->border_color.z, r->border_color.w
    };
    
    float border_positions[] = {
      r->top_left.x,     r->top_left.y,
      r->bottom_right.x, r->top_left.y,
      r->bottom_right.x, r->bottom_right.y,
      r->top_left.x,     r->bottom_right.y,
      r->top_left.x,     r->top_left.y
    };
    
    float border_texcoord[] = {
      0, 0,  0, 0,  0, 0,  0, 0,  0, 0
    };
    
    shader_program_enable_attribute(program_ui, "vPosition", 2, 2, border_positions);
    shader_program_enable_attribute(program_ui, "vTexcoord", 2, 2, border_texcoord);
    shader_program_enable_attribute(program_ui, "vColor", 4, 4, border_colors);
      
      glDrawArrays(GL_LINE_STRIP, 0, 5);
      
    shader_program_disable_attribute(program_ui, "vPosition");
    shader_program_disable_attribute(program_ui, "vTexcoord");
    shader_program_disable_attribute(program_ui, "vColor");
  
    glLineWidth(1);
  }
  
  glDisable(GL_BLEND);
  
  shader_program_disable(program_ui);
  
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
