#include <math.h>

#include "error.h"

#include "terrain.h"

void terrain_chunk_delete(terrain_chunk* tc) {
  
  glDeleteBuffers(3, tc->index_buffers);
  glDeleteBuffers(1, &tc->vertex_buffer);
  
  if (tc->offset_map != NULL) {
    texture_delete(tc->offset_map);
  }
  
  free(tc);
}

terrain* raw_load_file(char* filename) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "rb");
  
  if (!file) {
    error("Could not load file %s\n", filename);
  }
  
  long num_bytes = SDL_RWseek(file,0,SEEK_END);
  char* raw_bytes = malloc(num_bytes);
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, raw_bytes, num_bytes, 1);
  
  uint16_t* pixels = (uint16_t*)raw_bytes;
  
  /* Two bytes per pixel. Grayscale image. Assume square. */
  long num_pixels = num_bytes / 2;
  int width = sqrt(num_pixels);
  int height = width;
  
  const int CHUNK_SIZE = 32;
  
  terrain* ter = malloc(sizeof(terrain));
  ter->width = width;
  ter->height = height;
  ter->chunk_width = CHUNK_SIZE;
  ter->chunk_height = CHUNK_SIZE;
  ter->num_cols = (ter->width / ter->chunk_width);
  ter->num_rows = (ter->height / ter->chunk_height);
  ter->num_chunks = ter->num_cols * ter->num_rows;
  ter->heightmap = malloc(sizeof(float) * width * height);
  
  const float MAX_HEIGHT = 175;
  
  for(int i = 0; i < width * height; i++) {
    ter->heightmap[i] = (double)pixels[i] / (65536.0 / MAX_HEIGHT);
  }
  
  ter->chunks = malloc(sizeof(terrain_chunk*) * ter->num_chunks);
  
  for(int i = 0; i < ter->num_chunks; i++) {
    
    terrain_chunk* tc = malloc(sizeof(terrain_chunk));
    tc->id = i;
    tc->x = i % ter->num_cols;
    tc->y = i / ter->num_cols;
    tc->width = ter->chunk_width;
    tc->height = ter->chunk_height;
    tc->num_verts = (ter->chunk_width + 1) * (ter->chunk_height + 1);
    
    
    
    /* Buffer format: vec3 position, vec3 normal */
    float* vertex_buffer = malloc(sizeof(float) * 6 * tc->num_verts);
    int index = 0;
    for(int x = 0; x < tc->width+1; x++)
    for(int y = 0; y < tc->height+1; y++) {
      int gx = tc->x * ter->chunk_width + x;
      int gy = tc->y * ter->chunk_height + y;
      
      int gxh = min(ter->width-3, gx);
      int gyh = min(ter->height-3, gy);
      
      float offset = ter->heightmap[gxh + gyh * ter->width];
      float offset_r = ter->heightmap[(gxh+1) + gyh * ter->width];
      float offset_d = ter->heightmap[gxh + (gyh+1) * ter->width];
      
      vector3 pos = v3(gx, offset, gy);
      vector3 pos_r = v3_sub(v3(gx+1, offset_r, gy), pos);
      vector3 pos_d = v3_sub(v3(gx, offset_d, gy+1), pos);
      
      vector3 normal = v3_neg(v3_normalize(v3_cross(pos_r, pos_d)));
      
      vertex_buffer[index] = pos.x; index++;
      vertex_buffer[index] = offset; index++;
      vertex_buffer[index] = pos.z; index++;
      
      vertex_buffer[index] = normal.x; index++;
      vertex_buffer[index] = normal.y; index++;
      vertex_buffer[index] = normal.z; index++;
    }
    
    glGenBuffers(1, &tc->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * tc->num_verts, vertex_buffer, GL_DYNAMIC_DRAW);
    free(vertex_buffer);
    
    glGenBuffers(NUM_TERRAIN_BUFFERS, tc->index_buffers);
    for(int j = 0; j < NUM_TERRAIN_BUFFERS; j++) {
    
      int off = pow(2, j);
      tc->num_indicies[j] = (ter->chunk_width / off) * (ter->chunk_height / off) * 6;
      
      int* index_buffer = malloc(sizeof(int) * tc->num_indicies[j]);
      index = 0;
      for(int x = 0; x < tc->width; x+=off)
      for(int y = 0; y < tc->height; y+=off) {
        index_buffer[index] =  x +  y * (tc->width+1); index++;
        index_buffer[index] = (x+off) +  y * (tc->width+1); index++;
        index_buffer[index] = (x+off) + (y+off) * (tc->width+1); index++;
        index_buffer[index] =  x +  y * (tc->width+1); index++;
        index_buffer[index] = (x+off) + (y+off) * (tc->width+1); index++;
        index_buffer[index] =  x + (y+off) * (tc->width+1); index++;
      }
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[j]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * tc->num_indicies[j], index_buffer, GL_DYNAMIC_DRAW);
      free(index_buffer);
    }
    
    tc->offset_map = NULL;
    
    ter->chunks[i] = tc;
  }
  
  for(int i = 0; i < ter->num_chunks; i++) {
    int x = i % ter->num_cols;
    int y = i / ter->num_cols;
    
    ter->chunks[i]->left   = terrain_get_chunk(ter, x-1, y);
    ter->chunks[i]->right  = terrain_get_chunk(ter, x+1, y);
    ter->chunks[i]->top    = terrain_get_chunk(ter, x, y-1);
    ter->chunks[i]->bottom = terrain_get_chunk(ter, x, y+1);
  }
  
  SDL_RWclose(file);
  
  free(pixels);
  
  return ter;
  
}

terrain_chunk* terrain_get_chunk(terrain* ter, int x, int y) {
  
  if ((x < 0) || (y < 0) || (x >= ter->num_cols) || (y >= ter->num_cols)) {
    return NULL;
  }
  
  return ter->chunks[x + y * ter->num_cols];
  
}

void terrain_delete(terrain* ter) {
  
  for(int i = 0; i < ter->num_chunks; i++) {
    terrain_chunk_delete(ter->chunks[i]);
  }
  
  free(ter->heightmap);
  free(ter->chunks);
  free(ter);
  
}

static void check_position_in_bounds(terrain* ter, vector2 position) {
  
  if (position.x < 0) { error("Out of terrain bounds!"); }
  if (position.y < 0) { error("Out of terrain bounds!"); }
  
  if (position.x >= ter->width) { error("Out of terrain bounds!"); }
  if (position.y >= ter->height) { error("Out of terrain bounds!"); }
  
}

float terrain_height(terrain* ter, vector2 position) {
  
  vector2 amount = v2_fmod(position, 1.0);
  
  vector2 top_left = v2(floor(position.x), floor(position.y));
  vector2 top_right = v2(ceil(position.x), floor(position.y));
  vector2 bot_left = v2(floor(position.x), ceil(position.y));
  vector2 bot_right = v2(ceil(position.x), ceil(position.y));
  
  check_position_in_bounds(ter, top_left);
  check_position_in_bounds(ter, top_right);
  check_position_in_bounds(ter, bot_left);
  check_position_in_bounds(ter, bot_right);
  
  float s0 = ter->heightmap[(int)top_left.x + (int)top_left.y * ter->width];
  float s1 = ter->heightmap[(int)top_right.x + (int)top_right.y * ter->width];
  float s2 = ter->heightmap[(int)bot_left.x + (int)bot_left.y * ter->width];
  float s3 = ter->heightmap[(int)bot_right.x + (int)bot_right.y * ter->width];
  
  return bilinear_interpolation(s1, s0, s3, s2, amount.x, amount.y);
  
}