#include <string.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "camera.h"
#include "matrix.h"
#include "geometry.h"
#include "font.h"
#include "texture.h"

#include "text_renderer.h"

render_text* render_text_new(char* string, int buffersize, font* text_font) {
  
  render_text* rt = malloc( sizeof(render_text) );
  
  rt->string = malloc(buffersize + 1);
  rt->buffersize = buffersize;
  strcpy(rt->string, string);
  
  rt->font = text_font;
  rt->position = v2(0.0,0.0);
  rt->scale = v2(1.0,1.0);
  rt->color = v4_one();
  
  rt->alignment = text_align_left;
  rt->line_spacing = 0.0;
  rt->char_spacing = 0.0;
  rt->rotation = 0.0;
  
  rt->vert_positions = malloc(sizeof(float) * buffersize * 12);
  rt->vert_texcoords = malloc(sizeof(float) * buffersize * 8);
  
  render_text_update(rt);
  
  return rt;
};

void render_text_delete(render_text* rt) {
  
  free(rt->string);
  free(rt->vert_positions);
  free(rt->vert_texcoords);
  
  free(rt);
  
};

void render_text_update_buffer(render_text* rt, int buffersize) {
  
  rt->string = realloc(rt->string, buffersize+1);

  rt->vert_positions = realloc(rt->vert_positions, buffersize * 12);
  rt->vert_texcoords = realloc(rt->vert_texcoords, buffersize * 8);
  
};

void render_text_update_string(render_text* rt, char* string) {
  
  strcpy(rt->string,string);
  render_text_update(rt);
  
};

/*
  TODO:
    - alignment
    - rotation
*/
void render_text_update(render_text* rt) {
  
  font* f = rt->font;
  
  int pos_i = 0;
  int uv_i = 0;
  
  float x = rt->position.x;
  float y = -rt->position.y;
  
  int i = 0;
  while( rt->string[i] != '\0'){
  
    int ord = (int)rt->string[i];
    
    /* Texcoords */
    
    rt->vert_texcoords[uv_i] = f->locations[ord].x; uv_i++;
    rt->vert_texcoords[uv_i] = f->locations[ord].y; uv_i++;
    
    rt->vert_texcoords[uv_i] = f->locations[ord].x + f->sizes[ord].x; uv_i++;
    rt->vert_texcoords[uv_i] = f->locations[ord].y; uv_i++;
    
    rt->vert_texcoords[uv_i] = f->locations[ord].x + f->sizes[ord].x; uv_i++;
    rt->vert_texcoords[uv_i] = f->locations[ord].y + f->sizes[ord].y; uv_i++;
    
    rt->vert_texcoords[uv_i] = f->locations[ord].x; uv_i++;
    rt->vert_texcoords[uv_i] = f->locations[ord].y + f->sizes[ord].y; uv_i++;
    
    /* Positions */
    
    float o_x = x + (f->offsets[ord].x  * rt->scale.x);
    float o_y = y - (f->offsets[ord].y  * rt->scale.y);
    
    rt->vert_positions[pos_i] = o_x; pos_i++;
    rt->vert_positions[pos_i] = o_y; pos_i++;
    rt->vert_positions[pos_i] = 0; pos_i++;
    
    rt->vert_positions[pos_i] = o_x + (f->sizes[ord].x * rt->scale.x); pos_i++;
    rt->vert_positions[pos_i] = o_y; pos_i++;
    rt->vert_positions[pos_i] = 0; pos_i++;
    
    rt->vert_positions[pos_i] = o_x + (f->sizes[ord].x * rt->scale.x); pos_i++;
    rt->vert_positions[pos_i] = o_y - (f->sizes[ord].y * rt->scale.y); pos_i++;
    rt->vert_positions[pos_i] = 0; pos_i++;
    
    rt->vert_positions[pos_i] = o_x; pos_i++;
    rt->vert_positions[pos_i] = o_y - (f->sizes[ord].y * rt->scale.y); pos_i++;
    rt->vert_positions[pos_i] = 0; pos_i++;
    
    x = o_x + (f->sizes[ord].x * rt->scale.x);
    x += rt->char_spacing;
    
    if (ord == (int)' ') { x += rt->scale.x / 75; }
    
    if (ord == (int)'\n') { x = rt->position.x ; y -= rt->scale.y / 15; }
    
    i++;
  }
  
};

void render_text_render(render_text* rt) {

  glDisable(GL_CULL_FACE);

  glUseProgramObjectARB(0);
  
  /* Setup 2D camera */
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  /* Bind font texture */
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *(rt->font->texture_map) );
 
  /* Disable lighting, enable blending */
  
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor4f(1.0,1.0,1.0,1.0);
  glColor4f(rt->color.w, rt->color.x, rt->color.y, rt->color.z);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
      glVertexPointer(3, GL_FLOAT, 0, rt->vert_positions);
      glTexCoordPointer(2, GL_FLOAT, 0, rt->vert_texcoords);
      
      glDrawArrays(GL_QUADS, 0, strlen(rt->string) * 4);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  /* Set Back */
  
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glColor4f(1.0,1.0,1.0,1.0);
  
};