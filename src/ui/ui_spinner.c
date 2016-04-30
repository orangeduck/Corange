#include "ui/ui_spinner.h"
#include "ui/ui_style.h"

#include "cgraphics.h"

#include "assets/material.h"
#include "assets/texture.h"

ui_spinner* ui_spinner_new() {
  ui_spinner* s = malloc(sizeof(ui_spinner));
  s->top_left = vec2_zero();
  s->bottom_right = vec2_new(32, 32);
  s->color = vec4_black();
  s->texture = asset_hndl_new_load(ui_style_current->spinner_image);
  s->speed = ui_style_current->spinner_speed;
  s->rotation = 0;
  s->active = true;
  return s;
}

void ui_spinner_delete(ui_spinner* s) {
  free(s);
}

void ui_spinner_event(ui_spinner* s, SDL_Event e) {

}

void ui_spinner_update(ui_spinner* s) {
  s->rotation += s->speed * frame_time();
}

void ui_spinner_render(ui_spinner* s) {
  
  if (!s->active) return;
  
  vec2 top_left = s->top_left;
  vec2 top_right = vec2_new(s->bottom_right.x, s->top_left.y);
  vec2 bot_left = vec2_new(s->top_left.x, s->bottom_right.y);
  vec2 bot_right = s->bottom_right;
  
  vec2 center;
  center.x = (top_left.x + top_right.x) / 2;
  center.y = (top_left.y + bot_left.y) / 2;
  
  top_left = vec2_sub(top_left, center);
  top_right = vec2_sub(top_right, center);
  bot_left = vec2_sub(bot_left, center);
  bot_right = vec2_sub(bot_right, center);
  
  mat2 rot = mat2_rotation(s->rotation);
  
  top_left = mat2_mul_vec2(rot, top_left);
  top_right = mat2_mul_vec2(rot, top_right);
  bot_left = mat2_mul_vec2(rot, bot_left);
  bot_right = mat2_mul_vec2(rot, bot_right);
  
  top_left = vec2_add(top_left, center);
  top_right = vec2_add(top_right, center);
  bot_left = vec2_add(bot_left, center);
  bot_right = vec2_add(bot_right, center);
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  asset_hndl mat = asset_hndl_new_load(P("$CORANGE/shaders/ui.mat"));  
  shader_program* program_ui = material_get_entry(asset_hndl_ptr(&mat), 0)->program;
  
  shader_program_enable(program_ui);  
  shader_program_set_mat4(program_ui, "world", mat4_id());
  shader_program_set_mat4(program_ui, "view", mat4_id());
  shader_program_set_mat4(program_ui, "proj", mat4_orthographic(0, width, height, 0, -1, 1));
  shader_program_set_float(program_ui, "glitch", 0);
  shader_program_set_float(program_ui, "time", 0);
  shader_program_set_texture(program_ui, "diffuse", 0, s->texture);
  shader_program_set_texture(program_ui, "random",  1, asset_hndl_new_load(P("$CORANGE/textures/random.dds")));  
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  float spinner_color[] = {
    s->color.x, s->color.y, s->color.z, s->color.w,
    s->color.x, s->color.y, s->color.z, s->color.w,
    s->color.x, s->color.y, s->color.z, s->color.w,
    s->color.x, s->color.y, s->color.z, s->color.w
  };
  
  float spinner_position[] = {
    top_left.x, top_left.y,
    bot_left.x, bot_left.y,
    bot_right.x, bot_right.y,
    top_right.x, top_right.y
  };
  
  float spinner_texcoord[] = {
    0, 1, 1, 1, 1, 0, 0, 0
  };
  
  shader_program_enable_attribute(program_ui, "vPosition", 2, 2, spinner_position);
  shader_program_enable_attribute(program_ui, "vTexcoord", 2, 2, spinner_texcoord);
  shader_program_enable_attribute(program_ui, "vColor", 4, 4, spinner_color);
  
    glDrawArrays(GL_QUADS, 0, 4);
  
  shader_program_disable_attribute(program_ui, "vPosition");
  shader_program_disable_attribute(program_ui, "vTexcoord");
  shader_program_disable_attribute(program_ui, "vColor");
  
  glDisable(GL_BLEND);
  
  shader_program_disable(program_ui);
  
}
