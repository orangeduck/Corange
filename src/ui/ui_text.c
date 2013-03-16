#include "ui/ui_text.h"

#include "cgraphics.h"

#include "assets/material.h"
#include "assets/shader.h"
#include "assets/texture.h"
#include "assets/font.h"

ui_text* ui_text_new() {

  ui_text* t = malloc(sizeof(ui_text));
  
  t->string = malloc(strlen("")+1);
  strcpy(t->string, "");
  
  glGenBuffers(1, &t->positions_buffer);
  glGenBuffers(1, &t->texcoords_buffer);
  glGenBuffers(1, &t->colors_buffer);
  
  t->num_positions = 0;
  t->num_texcoords = 0;
  t->top_left = vec2_zero();
  t->bottom_right = vec2_zero();
  
  t->font = asset_hndl_new_load(P("$CORANGE/fonts/console_font.fnt"));
  
  t->position = vec2_new(0.0,0.0);
  t->scale = vec2_new(1.0,1.0);
  t->color = vec4_black();
  
  t->halign = text_align_left;
  t->valign = text_align_top;
  t->line_spacing = 0.0;
  t->char_spacing = 0.0;
  t->rotation = 0.0;
  
  t->line_length = 0.0;
  
  t->active = true;
  
  ui_text_draw(t);
  
  return t;

}

ui_text* ui_text_new_string(char* string) {
  
  ui_text* t = ui_text_new();
  ui_text_draw_string(t, string);
  
  return t;
}

void ui_text_delete(ui_text* t) {
  
  free(t->string);

  glDeleteBuffers(1, &t->positions_buffer);
  glDeleteBuffers(1, &t->texcoords_buffer);
  glDeleteBuffers(1, &t->colors_buffer);
  
  free(t);
  
}

void ui_text_move(ui_text* t, vec2 pos) {
  t->position = pos;
  ui_text_draw(t);
}

void ui_text_set_font(ui_text* t, asset_hndl font) {
  t->font = font;
  ui_text_draw(t);
}

void ui_text_set_color(ui_text* t, vec4 color) {
  t->color = color;
  ui_text_draw(t);
}

void ui_text_set_scale(ui_text* t, vec2 scale) {
  t->scale = scale;
  ui_text_draw(t);
}

void ui_text_align(ui_text* t, int halign, int valign) {
  t->halign = halign;
  t->valign = valign;
}

void ui_text_event(ui_text* t, SDL_Event e) {

}

void ui_text_update(ui_text* t) {

}

void ui_text_draw_string(ui_text* t, char* string) {
  
  t->string = realloc(t->string, strlen(string) + 1);
  strcpy(t->string, string);
  ui_text_draw(t);
  
}

static int ui_text_charcount(ui_text* t) {
  
  int count = 0;
  int i = 0;
  while (t->string[i] != '\0') {
  
    char c = t->string[i];
    if ((c != ' ') && 
        (c != '\0') && 
        (c != '\t') && 
        (c != '\n')) {
      count++;
    }
  
    i++;
  }
  
  return count;
}

void ui_text_draw(ui_text* t) {
  
  const float base_scale = 250;
  
  font* f = asset_hndl_ptr(&t->font);
  
  int pos_i = 0;
  int uv_i = 0;
  
  float x = t->position.x;
  float y = t->position.y;
  
  float space_length = 0.025 * t->scale.x * base_scale;
  float tab_length = space_length * 4;
  float newline_height = 0.06 * t->scale.y * base_scale + t->line_spacing;
  
  int charcount = ui_text_charcount(t);
  vec2* vert_texcoords = malloc(sizeof(vec2) * charcount * 4);
  vec2* vert_positions = malloc(sizeof(vec2) * charcount * 4);
  vec4* vert_colors    = malloc(sizeof(vec4) * charcount * 4);
  
  int newline_at = 0;
  
  for (int i = 0; i < strlen(t->string)+1; i++) {
  
    uint8_t ord = t->string[i];
    
    if ((ord == '\0') ||
        (ord == '\n') ||     
        ((ord == ' ') && 
         (t->line_length != 0.0) &&
         (x - t->position.x > t->line_length)) ) {
      
      if (t->halign == text_align_center) {
        float total_length = x - t->position.x - t->char_spacing;
        float offset_x = total_length / 2;
        
        int j = newline_at;
        while( j < pos_i ) {
          vert_positions[j].x -= offset_x; j++;
          vert_positions[j].x -= offset_x; j++;
          vert_positions[j].x -= offset_x; j++;
          vert_positions[j].x -= offset_x; j++;
        }
      }
      
      newline_at = pos_i;
      x = t->position.x;
      y += newline_height;
      continue;
    }
    
    if (ord == ' ') {
      x += space_length;
      x += t->char_spacing;
      continue;
    }
    
    if (ord == '\t') {
      x += tab_length;
      x += t->char_spacing;
      continue;
    }
    
    /* Texcoords */
  
    vert_texcoords[uv_i] = f->locations[ord]; uv_i++;
    vert_texcoords[uv_i] = vec2_add(f->locations[ord], vec2_new(f->sizes[ord].x, 0)); uv_i++;
    vert_texcoords[uv_i] = vec2_add(f->locations[ord], f->sizes[ord]); uv_i++;
    vert_texcoords[uv_i] = vec2_add(f->locations[ord], vec2_new(0, f->sizes[ord].y)); uv_i++;
    
    /* Positions */
    
    float o_x = x + (f->offsets[ord].x * t->scale.x * base_scale);
    float o_y = y + (f->offsets[ord].y * t->scale.y * base_scale);
    float s_x = (f->sizes[ord].x * t->scale.x * base_scale);
    float s_y = (f->sizes[ord].y * t->scale.y * base_scale);
    
    vert_positions[pos_i] = vec2_new(o_x, o_y); pos_i++;
    vert_positions[pos_i] = vec2_new(o_x + s_x, o_y); pos_i++;
    vert_positions[pos_i] = vec2_new(o_x + s_x, o_y + s_y); pos_i++;
    vert_positions[pos_i] = vec2_new(o_x, o_y + s_y); pos_i++;
    
    x = o_x + s_x + t->char_spacing;
    
  }
  
  t->num_positions = pos_i;
  t->num_texcoords = uv_i;
  
  float total_length = x - t->position.x;
  float total_height = y - t->position.y + newline_height;
  
  float offset_x = 0;
  float offset_y = 0;
  
  if (t->halign == text_align_right) {
    offset_x = total_length;
  }
  
  if (t->valign == text_align_bottom) {
    offset_y = total_height;
  } else if (t->valign == text_align_center) {
    offset_y = total_height / 2;
  }
    
  if ((offset_x != 0) || (offset_y != 0)) {
    int pos_i = 0;
    while( pos_i < t->num_positions ) {
      
      for(int j = 0; j < 4; j++) {
        vert_positions[pos_i] = vec2_sub(vert_positions[pos_i], vec2_new(offset_x, offset_y)); pos_i++;
      }
    }
  }
  
  if (t->rotation != 0) {
    
    mat2 rot = mat2_rotation(t->rotation);
    
    int i = 0;
    while ( i * 4 < t->num_positions ) {
      
      vec2 v1_pos = vert_positions[i * 4 + 0];
      vec2 v2_pos = vert_positions[i * 4 + 1];
      vec2 v3_pos = vert_positions[i * 4 + 2];
      vec2 v4_pos = vert_positions[i * 4 + 3];
      
      v1_pos = vec2_sub(v1_pos, t->position);
      v2_pos = vec2_sub(v2_pos, t->position);
      v3_pos = vec2_sub(v3_pos, t->position);
      v4_pos = vec2_sub(v4_pos, t->position);
      
      v1_pos = mat2_mul_vec2(rot, v1_pos);
      v2_pos = mat2_mul_vec2(rot, v2_pos);
      v3_pos = mat2_mul_vec2(rot, v3_pos);
      v4_pos = mat2_mul_vec2(rot, v4_pos);
      
      v1_pos = vec2_add(v1_pos, t->position);
      v2_pos = vec2_add(v2_pos, t->position);
      v3_pos = vec2_add(v3_pos, t->position);
      v4_pos = vec2_add(v4_pos, t->position);
      
      vert_positions[i * 4 + 0] = v1_pos;
      vert_positions[i * 4 + 1] = v2_pos;
      vert_positions[i * 4 + 2] = v3_pos;
      vert_positions[i * 4 + 3] = v4_pos;
      
      i++;
    }
  
  }
  
  /* Find Bounds */
  
  t->top_left.x = FLT_MAX;
  t->top_left.y = FLT_MAX;
  t->bottom_right.x = 0;
  t->bottom_right.y = 0;
  
  for ( int i = 0; i < t->num_positions; i++ ) {
      t->top_left.x = min(t->top_left.x, vert_positions[i].x); 
      t->top_left.y = min(t->top_left.y, vert_positions[i].y); 
      t->bottom_right.x = max(t->bottom_right.x, vert_positions[i].x);      
      t->bottom_right.y = max(t->bottom_right.y, vert_positions[i].y);
  }
  
  for (int i = 0; i < charcount * 4; i++) {
    vert_colors[i] = t->color;
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, t->colors_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * charcount * 4, vert_colors, GL_DYNAMIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, t->texcoords_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * charcount * 4, vert_texcoords, GL_DYNAMIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, t->positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * charcount * 4, vert_positions, GL_DYNAMIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  free(vert_colors);
  free(vert_texcoords);
  free(vert_positions);
  
}

void ui_text_render(ui_text* t) {
  
  if(!t->active) { return; }
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  asset_hndl mat = asset_hndl_new_load(P("$CORANGE/shaders/ui.mat"));  
  shader_program* program_ui = material_get_entry(asset_hndl_ptr(&mat), 0)->program;
  
  shader_program_enable(program_ui);  
  shader_program_set_mat4(program_ui, "world", mat4_id());
  shader_program_set_mat4(program_ui, "view", mat4_id());
  shader_program_set_mat4(program_ui, "proj", mat4_orthographic(0, width, height, 0, -1, 1));
  shader_program_set_float(program_ui, "time", 0);
  shader_program_set_float(program_ui, "glitch", 0);
  
  font* font = asset_hndl_ptr(&t->font);
  
  shader_program_set_texture(program_ui, "diffuse", 0, font->texture_map);
  shader_program_set_texture(program_ui, "random",  1, asset_hndl_new_load(P("$CORANGE/textures/random.dds")));
  
  glBindBuffer(GL_ARRAY_BUFFER, t->positions_buffer);
  shader_program_enable_attribute(program_ui, "vPosition", 2, 2, NULL);
  
  glBindBuffer(GL_ARRAY_BUFFER, t->texcoords_buffer);
  shader_program_enable_attribute(program_ui, "vTexcoord", 2, 2, NULL);
  
  glBindBuffer(GL_ARRAY_BUFFER, t->colors_buffer);
  shader_program_enable_attribute(program_ui, "vColor", 4, 4, NULL);
    
    glDrawArrays(GL_QUADS, 0, ui_text_charcount(t) * 4);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  shader_program_disable_attribute(program_ui, "vPosition");
  shader_program_disable_attribute(program_ui, "vTexcoord");
  shader_program_disable_attribute(program_ui, "vColor");
  shader_program_disable(program_ui);
  
  glDisable(GL_BLEND);
  
}

bool ui_text_contains_point(ui_text* t, vec2 position) {
  
  if(!t->active) { return false; }
  
  if(( position.x > t->top_left.x ) && ( position.x < t->bottom_right.x ) &&
     ( position.y > t->top_left.y ) && ( position.y < t->bottom_right.y )) {
    return true;
  } else {
    return false;
  }
  
}
