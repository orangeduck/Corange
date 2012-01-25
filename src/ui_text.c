#include <string.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "font.h"
#include "viewport.h"
#include "texture.h"
#include "matrix.h"

#include "asset_manager.h"

#include "ui_text.h"

ui_text* ui_text_new() {

  ui_text* text = malloc( sizeof(ui_text) );
  
  text->string = malloc(strlen("New Text") + 1);
  text->buffersize = strlen("New Text") + 1;
  strcpy(text->string, "New Text");
  
  text->font = asset_load_get("$CORANGE/fonts/console_font.fnt");
  text->position = v2(0.0,0.0);
  text->scale = v2(1.0,1.0);
  text->color = v4_black();
  
  text->alignment = text_align_left;
  text->line_spacing = 0.0;
  text->char_spacing = 0.0;
  text->rotation = 0.0;
  
  text->active = true;
  
  glGenBuffers(1, &text->positions_buffer);
  glGenBuffers(1, &text->texcoords_buffer);
  
  ui_text_update_properties(text);
  
  return text;

}

ui_text* ui_text_new_string(char* string, font* text_font) {
  
  ui_text* text = malloc( sizeof(ui_text) );
  
  int buffersize = strlen(string) + 1;
  
  text->string = malloc(buffersize);
  text->buffersize = buffersize;
  strcpy(text->string, string);
  
  text->font = text_font;
  text->position = v2(0.0,0.0);
  text->scale = v2(1.0,1.0);
  text->color = v4_black();
  
  text->alignment = text_align_left;
  text->line_spacing = 0.0;
  text->char_spacing = 0.0;
  text->rotation = 0.0;
  
  glGenBuffers(1, &text->positions_buffer);
  glGenBuffers(1, &text->texcoords_buffer);
  
  ui_text_update_properties(text);
  
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

void ui_text_update_string(ui_text* text, char* string) {
  
  int buflen = strlen(string) + 1;
  
  if( buflen > text->buffersize ) {
    text->string = realloc(text->string, buflen);
    text->buffersize = buflen;
  }
  
  strcpy(text->string, string);
  ui_text_update_properties(text);
  
}

void ui_text_update_properties(ui_text* text) {
  
  const float base_scale = 250;
  
  font* f = text->font;
  
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
      
      if (text->alignment == text_align_center) {
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
  
  if (text->alignment == text_align_right) {
    offset_x = total_length;
  }
  
  if (text->vertical_alignment == text_align_bottom) {
    offset_y = total_height;
  } else if (text->vertical_alignment == text_align_center) {
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
    
    matrix_2x2 rot = m22_rotation(text->rotation);
    m22_print(rot);
    
    int i = 0;
    while( i * 12 < text->num_positions ) {
      
      vector2 v1_pos = v2(vert_positions[i * 12 + 0], vert_positions[i * 12 + 1]);
      vector2 v2_pos = v2(vert_positions[i * 12 + 3], vert_positions[i * 12 + 4]);
      vector2 v3_pos = v2(vert_positions[i * 12 + 6], vert_positions[i * 12 + 7]);
      vector2 v4_pos = v2(vert_positions[i * 12 + 9], vert_positions[i * 12 + 10]);
      
      v1_pos = v2_sub(v1_pos, text->position);
      v2_pos = v2_sub(v2_pos, text->position);
      v3_pos = v2_sub(v3_pos, text->position);
      v4_pos = v2_sub(v4_pos, text->position);
      
      v1_pos = m22_mul_v2(rot, v1_pos);
      v2_pos = m22_mul_v2(rot, v2_pos);
      v3_pos = m22_mul_v2(rot, v3_pos);
      v4_pos = m22_mul_v2(rot, v4_pos);
      
      v1_pos = v2_add(v1_pos, text->position);
      v2_pos = v2_add(v2_pos, text->position);
      v3_pos = v2_add(v3_pos, text->position);
      v4_pos = v2_add(v4_pos, text->position);
      
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
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glBindTexture(GL_TEXTURE_2D, *(text->font->texture_map) );
  glEnable(GL_TEXTURE_2D);
  
  glColor4f(text->color.r, text->color.g, text->color.b, text->color.a);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, text->positions_buffer);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, text->texcoords_buffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    
    glDrawArrays(GL_QUADS, 0, ui_text_charcount(text) * 4);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glColor4f(1.0,1.0,1.0,1.0);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}

bool ui_text_contains_position(ui_text* text, vector2 position) {
  
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
