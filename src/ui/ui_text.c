#include "ui/ui_text.h"

#include "cgraphics.h"

#include "assets/texture.h"
#include "assets/font.h"

ui_text* ui_text_new() {

  ui_text* text = malloc( sizeof(ui_text) );
  
  text->string = malloc(strlen("New Text") + 1);
  text->buffersize = strlen("New Text") + 1;
  strcpy(text->string, "New Text");
  
  glGenBuffers(1, &text->positions_buffer);
  glGenBuffers(1, &text->texcoords_buffer);
  
  text->num_positions = 0;
  text->num_texcoords = 0;
  text->top_left = vec2_zero();
  text->bottom_right = vec2_zero();
  
  text->font = asset_hndl_new_load(P("$CORANGE/fonts/console_font.fnt"));
  
  text->position = vec2_new(0.0,0.0);
  text->scale = vec2_new(1.0,1.0);
  text->color = vec4_black();
  
  text->halign = text_align_left;
  text->valign = text_align_top;
  text->line_spacing = 0.0;
  text->char_spacing = 0.0;
  text->rotation = 0.0;
  
  text->active = true;
  
  ui_text_draw(text);
  
  return text;

}

ui_text* ui_text_new_string(char* string) {
  
  ui_text* text = ui_text_new();
  ui_text_draw_string(text, string);
  
  return text;
}

void ui_text_delete(ui_text* text) {
  
  free(text->string);

  glDeleteBuffers(1, &text->positions_buffer);
  glDeleteBuffers(1, &text->texcoords_buffer);
  
  free(text);
  
}

void ui_text_update(ui_text* text) {

}

void ui_text_draw_string(ui_text* text, char* string) {
  
  int buflen = strlen(string) + 1;
  
  if( buflen > text->buffersize ) {
    text->string = realloc(text->string, buflen);
    text->buffersize = buflen;
  }
  
  strcpy(text->string, string);
  ui_text_draw(text);
  
}

void ui_text_draw(ui_text* text) {
  
  const float base_scale = 250;
  
  font* f = asset_hndl_ptr(text->font);
  
  int pos_i = 0;
  int uv_i = 0;
  
  float x = text->position.x;
  float y = text->position.y;
  
  float space_length = 0.025 * text->scale.x * base_scale;
  float tap_length = space_length * 4;
  float newline_height = 0.06 * text->scale.y * base_scale;
  
  float* vert_texcoords = malloc(sizeof(float) * text->buffersize * 8);
  float* vert_positions = malloc(sizeof(float) * text->buffersize * 12);
  
  int newline_at = 0;
  
  int i = 0;
  int ord;
  while(1){
  
    ord = (int)text->string[i];
    
    if (ord == (int)' ') {
    
      x += space_length;
      x += text->char_spacing;
    
    } else if (ord == (int)'\t') {
    
      x += tap_length;
      x += text->char_spacing;
    
    } else if ((ord == (int)'\n') || (ord == (int)'\0')) {
      
      if (text->halign == text_align_center) {
        float total_length = x - text->position.x;
        float offset_x = total_length / 2;
        
        int j = newline_at;
        while( j < pos_i ) {
          vert_positions[j] -= offset_x; j += 3;
          vert_positions[j] -= offset_x; j += 3;
          vert_positions[j] -= offset_x; j += 3;
          vert_positions[j] -= offset_x; j += 3;
        }
      }
      
      newline_at = pos_i;
      x = text->position.x;
      y += newline_height;
      
      if(ord == (int)'\0') break;
      
    } else {
    
    /* Texcoords */
    
    vert_texcoords[uv_i] = f->locations[ord].x; uv_i++;
    vert_texcoords[uv_i] = f->locations[ord].y; uv_i++;
    
    vert_texcoords[uv_i] = f->locations[ord].x + f->sizes[ord].x; uv_i++;
    vert_texcoords[uv_i] = f->locations[ord].y; uv_i++;
    
    vert_texcoords[uv_i] = f->locations[ord].x + f->sizes[ord].x; uv_i++;
    vert_texcoords[uv_i] = f->locations[ord].y + f->sizes[ord].y; uv_i++;
    
    vert_texcoords[uv_i] = f->locations[ord].x; uv_i++;
    vert_texcoords[uv_i] = f->locations[ord].y + f->sizes[ord].y; uv_i++;
    
    /* Positions */
    
    float o_x = x + (f->offsets[ord].x * text->scale.x * base_scale);
    float o_y = y + (f->offsets[ord].y * text->scale.y * base_scale);
    
    vert_positions[pos_i] = o_x; pos_i++;
    vert_positions[pos_i] = o_y; pos_i++;
    vert_positions[pos_i] = 0; pos_i++;
    
    vert_positions[pos_i] = o_x + (f->sizes[ord].x * text->scale.x * base_scale); pos_i++;
    vert_positions[pos_i] = o_y; pos_i++;
    vert_positions[pos_i] = 0; pos_i++;
    
    vert_positions[pos_i] = o_x + (f->sizes[ord].x * text->scale.x * base_scale); pos_i++;
    vert_positions[pos_i] = o_y + (f->sizes[ord].y * text->scale.y * base_scale); pos_i++;
    vert_positions[pos_i] = 0; pos_i++;
    
    vert_positions[pos_i] = o_x; pos_i++;
    vert_positions[pos_i] = o_y + (f->sizes[ord].y * text->scale.y * base_scale); pos_i++;
    vert_positions[pos_i] = 0; pos_i++;
    
    x = o_x + (f->sizes[ord].x * text->scale.x * base_scale);
    x += text->char_spacing;
      
    }
    
    i++;
  }
  
  text->num_positions = pos_i;
  text->num_texcoords = uv_i;
  
  float total_length = x - text->position.x;
  float total_height = y - text->position.y;
  
  float offset_x = 0;
  float offset_y = 0;
  
  if (text->halign == text_align_right) {
    offset_x = total_length;
  }
  
  if (text->valign == text_align_bottom) {
    offset_y = total_height;
  } else if (text->valign == text_align_center) {
    offset_y = total_height / 2;
  }
    
  if ((offset_x != 0) || (offset_y != 0)) {
  
    int i = 0;
    int pos_i = 0;
    while( pos_i < text->num_positions ) {
      
      for(int j = 0; j < 4; j++) {
        
        vert_positions[pos_i] -= offset_x; pos_i++;
        vert_positions[pos_i] -= offset_y; pos_i++;
        pos_i++;
        
        }
    
      i++;
    }
  }
  
  if (text->rotation > 0) {
    
    mat2 rot = mat2_rotation(text->rotation);
    mat2_print(rot);
    
    int i = 0;
    while( i * 12 < text->num_positions ) {
      
      vec2 v1_pos = vec2_new(vert_positions[i * 12 + 0], vert_positions[i * 12 + 1]);
      vec2 v2_pos = vec2_new(vert_positions[i * 12 + 3], vert_positions[i * 12 + 4]);
      vec2 v3_pos = vec2_new(vert_positions[i * 12 + 6], vert_positions[i * 12 + 7]);
      vec2 v4_pos = vec2_new(vert_positions[i * 12 + 9], vert_positions[i * 12 + 10]);
      
      v1_pos = vec2_sub(v1_pos, text->position);
      v2_pos = vec2_sub(v2_pos, text->position);
      v3_pos = vec2_sub(v3_pos, text->position);
      v4_pos = vec2_sub(v4_pos, text->position);
      
      v1_pos = mat2_mul_vec2(rot, v1_pos);
      v2_pos = mat2_mul_vec2(rot, v2_pos);
      v3_pos = mat2_mul_vec2(rot, v3_pos);
      v4_pos = mat2_mul_vec2(rot, v4_pos);
      
      v1_pos = vec2_add(v1_pos, text->position);
      v2_pos = vec2_add(v2_pos, text->position);
      v3_pos = vec2_add(v3_pos, text->position);
      v4_pos = vec2_add(v4_pos, text->position);
      
      vert_positions[i * 12 + 0] = v1_pos.x;
      vert_positions[i * 12 + 1] = v1_pos.y;
      vert_positions[i * 12 + 3] = v2_pos.x;
      vert_positions[i * 12 + 4] = v2_pos.y;
      vert_positions[i * 12 + 6] = v3_pos.x;
      vert_positions[i * 12 + 7] = v3_pos.y;
      vert_positions[i * 12 + 9] = v4_pos.x;
      vert_positions[i * 12 + 10] = v4_pos.y;
      
      i++;
    }
  
  }
  
  text->top_left.x = 99999;
  text->top_left.y = 99999;
  text->bottom_right.x = 0;
  text->bottom_right.y = 0;
  
  i = 0;
  pos_i = 0;
  while( pos_i < text->num_positions ) {
    
    for(int j = 0; j < 4; j++) {
    
      text->top_left.x = min(text->top_left.x, vert_positions[pos_i]); 
      text->bottom_right.x = max(text->bottom_right.x, vert_positions[pos_i]); pos_i++;
      
      text->top_left.y = min(text->top_left.y, vert_positions[pos_i]); 
      text->bottom_right.y = max(text->bottom_right.y, vert_positions[pos_i]); pos_i++;
      
      pos_i++;
    
    }
  
    i++;
  }
    
  glBindBuffer(GL_ARRAY_BUFFER, text->texcoords_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * text->buffersize * 8, vert_texcoords, GL_STATIC_READ);
  
  glBindBuffer(GL_ARRAY_BUFFER, text->positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * text->buffersize * 12, vert_positions, GL_STATIC_READ);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  free(vert_texcoords);
  free(vert_positions);
  
}

static int ui_text_charcount(ui_text* text) {
  
  int count = 0;
  int i = 0;
  while(text->string[i] != '\0') {
  
    char c = text->string[i];
    if ((c != ' ') && (c != '\0') && (c != '\t') && (c != '\n')) {
      count++;
    }
  
    i++;
  }
  
  
  return count;
}

void ui_text_render(ui_text* text) {
  
  if(!text->active) {
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
  
  font* font = asset_hndl_ptr(text->font);
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(font->texture_map)) );
  glEnable(GL_TEXTURE_2D);
  
  glColor4f(text->color.x, text->color.y, text->color.z, text->color.w);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, text->positions_buffer);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, text->texcoords_buffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    
    glDrawArrays(GL_QUADS, 0, ui_text_charcount(text) * 4);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glColor4f(1.0,1.0,1.0,1.0);
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}

bool ui_text_contains_position(ui_text* text, vec2 position) {
  
  if(!text->active) {
    return false;
  }
  
  if(( position.x > text->top_left.x ) && ( position.x < text->bottom_right.x ) &&
     ( position.y > text->top_left.y ) && ( position.y < text->bottom_right.y )) {
    return true;
  } else {
    return false;
  }
  
}
