#include "assets/terrain.h"

void terrain_chunk_delete(terrain_chunk* tc) {
  
  glDeleteBuffers(3, tc->index_buffers);
  glDeleteBuffers(1, &tc->vertex_buffer);
  
  free(tc);
}

static void terrain_new_chunk(terrain* ter, int i) {

  const int SUBDIVISIONS = NUM_TERRAIN_SUBDIVISIONS+1; 

  terrain_chunk* tc = malloc(sizeof(terrain_chunk));
  tc->id = i;
  tc->x = i % ter->num_cols;
  tc->y = i / ter->num_cols;
  tc->width = ter->chunk_width;
  tc->height = ter->chunk_height;
  
  int x_max = tc->width*SUBDIVISIONS+1;
  int y_max = tc->height*SUBDIVISIONS+1;
  
  tc->num_verts = x_max * y_max + x_max * 2 + y_max * 2;
  float* vertex_buffer = malloc(sizeof(float) * 12 * tc->num_verts);
  int index = 0;
  
  for(int x = 0; x < x_max; x++)
  for(int y = 0; y < y_max; y++) {
    float gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    float gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float offset = terrain_height(ter, vec2_new(gx, gy));
    vec3 pos = vec3_new(gx, offset, gy);
    
    vertex_buffer[index] = pos.x; index++;
    vertex_buffer[index] = pos.y; index++;
    vertex_buffer[index] = pos.z; index++;
    
    vec3 normal  = vec3_normalize(terrain_normal(ter, vec2_new(gx, gy)));
    vec3 tangent = vec3_normalize(vec3_cross(normal, vec3_new(1, 0, 0)));
    vec3 binorm  = vec3_normalize(vec3_cross(normal, tangent));
    
    vertex_buffer[index] = normal.x; index++;
    vertex_buffer[index] = normal.y; index++;
    vertex_buffer[index] = normal.z; index++;
    
    vertex_buffer[index] = tangent.x; index++;
    vertex_buffer[index] = tangent.y; index++;
    vertex_buffer[index] = tangent.z; index++;
    
    vertex_buffer[index] = binorm.x; index++;
    vertex_buffer[index] = binorm.y; index++;
    vertex_buffer[index] = binorm.z; index++;
  }
  
  /* Adding fins. Don't look, horrible code */
  
  const float FIN_DEPTH = 5.0;
  
  for(int y = 0; y < y_max; y++) {
    int gx = tc->x * ter->chunk_width + 0;
    int gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float offset = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    vec3 pos = vec3_new(gx, offset, gy);
    
    vertex_buffer[index] = pos.x; index++;
    vertex_buffer[index] = pos.y; index++;
    vertex_buffer[index] = pos.z; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
  }
  
  for(int y = 0; y < y_max; y++) {
    int gx = tc->x * ter->chunk_width + ter->chunk_width;
    int gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float offset = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    vec3 pos = vec3_new(gx, offset, gy);
    
    vertex_buffer[index] = pos.x; index++;
    vertex_buffer[index] = pos.y; index++;
    vertex_buffer[index] = pos.z; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
  }
  
  for(int x = 0; x < x_max; x++) {
    int  gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    int  gy = tc->y * ter->chunk_height + 0;
    
    float offset = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    vec3 pos = vec3_new(gx, offset, gy);
    
    vertex_buffer[index] = pos.x; index++;
    vertex_buffer[index] = pos.y; index++;
    vertex_buffer[index] = pos.z; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
  }
  
  for(int x = 0; x < x_max; x++) {
    int  gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    int  gy = tc->y * ter->chunk_height + ter->chunk_height;
    
    float offset = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    vec3 pos = vec3_new(gx, offset, gy);
    
    vertex_buffer[index] = pos.x; index++;
    vertex_buffer[index] = pos.y; index++;
    vertex_buffer[index] = pos.z; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 1; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 0; index++;
    vertex_buffer[index] = 1; index++;
  }
  
  glGenBuffers(1, &tc->vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * tc->num_verts, vertex_buffer, GL_STATIC_DRAW);
  free(vertex_buffer);
  
  glGenBuffers(NUM_TERRAIN_BUFFERS, tc->index_buffers);
  for(int j = 0; j < NUM_TERRAIN_BUFFERS; j++) {
  
    int off = pow(2, j);
    int x_max = tc->width * SUBDIVISIONS;
    int y_max = tc->height * SUBDIVISIONS;
    
    tc->num_indicies[j] = (x_max / off) * (y_max / off) * 6 + (x_max / off) * 12 + (y_max / off) * 12;
    
    int* index_buffer = malloc(sizeof(int) * tc->num_indicies[j]);
    index = 0;
    
    for(int x = 0; x < x_max; x+=off)
    for(int y = 0; y < y_max; y+=off) {
      index_buffer[index] =  x +  y * (x_max+1); index++;
      index_buffer[index] = (x+off) +  y * (x_max+1); index++;
      index_buffer[index] = (x+off) + (y+off) * (x_max+1); index++;
      index_buffer[index] =  x +  y * (x_max+1); index++;
      index_buffer[index] = (x+off) + (y+off) * (x_max+1); index++;
      index_buffer[index] =  x + (y+off) * (x_max+1); index++;
    }
    
    /* Again, adding fins. Don't look horrible code */
    
    int x_base = (x_max + 1) * (y_max + 1);
    int y_base = (x_max + 1) * (y_max + 1) + (x_max + 1) * 2;
    
    for(int x = 0; x < x_max; x+=off) {
      index_buffer[index] = x + 0 * (x_max+1); index++;
      index_buffer[index] =  x_base + x; index++;
      index_buffer[index] = (x+off) + 0 * (x_max+1); index++;
      
      index_buffer[index] = (x+off) + 0 * (x_max+1); index++;
      index_buffer[index] = x_base + x; index++;
      index_buffer[index] = x_base + x+off; index++;
    }
    
    for(int x = 0; x < x_max; x+=off) {
      index_buffer[index] = x + y_max * (x_max+1); index++;
      index_buffer[index] = (x+off) + y_max * (x_max+1); index++;
      index_buffer[index] =  x_base + y_max+1 + x; index++;
      
      index_buffer[index] = (x+off) + y_max * (x_max+1); index++;
      index_buffer[index] = x_base + x_max+1 + x+off; index++;
      index_buffer[index] = x_base + x_max+1 + x; index++;
    }
    
    for(int y = 0; y < y_max; y+=off) {
      index_buffer[index] = 0 + y * (x_max+1); index++;
      index_buffer[index] = 0 + (y+off) * (x_max+1); index++;
      index_buffer[index] = y_base + y; index++;
      
      index_buffer[index] = 0 + (y+off) * (x_max+1); index++;
      index_buffer[index] = y_base + y+off; index++;
      index_buffer[index] = y_base + y; index++;
    }
    
    for(int y = 0; y < y_max; y+=off) {
      index_buffer[index] = x_max + y * (x_max+1); index++;
      index_buffer[index] = y_base + y_max+1 + y; index++;
      index_buffer[index] = x_max + (y+off) * (x_max+1); index++;
      
      index_buffer[index] = x_max + (y+off) * (x_max+1); index++;
      index_buffer[index] = y_base + y_max+1 + y; index++;
      index_buffer[index] = y_base + y_max+1 + y+off; index++;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[j]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * tc->num_indicies[j], index_buffer, GL_DYNAMIC_DRAW);
    free(index_buffer);
  }
  
  ter->chunks[i] = tc;

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
  
  const float MAX_HEIGHT = 128;
  
  for(int i = 0; i < width * height; i++) {
    ter->heightmap[i] = (double)pixels[i] / (65536.0 / MAX_HEIGHT);
  }
  
  ter->chunks = malloc(sizeof(terrain_chunk*) * ter->num_chunks);
  
  for(int i = 0; i < ter->num_chunks; i++) {
    terrain_new_chunk(ter, i);
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

float terrain_height(terrain* ter, vec2 position) {
  
  vec2 amount = vec2_fmod(position, 1.0);
  
  vec2 top_left = vec2_new(floor(position.x), floor(position.y));
  vec2 top_right = vec2_new(ceil(position.x), floor(position.y));
  vec2 bot_left = vec2_new(floor(position.x), ceil(position.y));
  vec2 bot_right = vec2_new(ceil(position.x), ceil(position.y));
  
  top_left.x = clamp(top_left.x, 0, ter->width-1);
  top_left.y = clamp(top_left.y, 0, ter->height-1);
  top_right.x = clamp(top_right.x, 0, ter->width-1);
  top_right.y = clamp(top_right.y, 0, ter->height-1);
  bot_left.x = clamp(bot_left.x, 0, ter->width-1);
  bot_left.y = clamp(bot_left.y, 0, ter->height-1);
  bot_right.x = clamp(bot_right.x, 0, ter->width-1);
  bot_right.y = clamp(bot_right.y, 0, ter->height-1);
  
  float s0 = ter->heightmap[(int)top_left.x + (int)top_left.y * ter->width];
  float s1 = ter->heightmap[(int)top_right.x + (int)top_right.y * ter->width];
  float s2 = ter->heightmap[(int)bot_left.x + (int)bot_left.y * ter->width];
  float s3 = ter->heightmap[(int)bot_right.x + (int)bot_right.y * ter->width];
  
  return bilinear_interp(s1, s0, s3, s2, amount.x, amount.y);
  
}

vec3 terrain_normal(terrain* ter, vec2 position) {
  
  float base = terrain_height(ter, position);
  float base_x = terrain_height(ter, vec2_add(position, vec2_new(1,0)));
  float base_y = terrain_height(ter, vec2_add(position, vec2_new(0,-1)));
  
  vec3 basev = vec3_new(position.x, base, position.y);
  vec3 base_xv = vec3_new(position.x+1, base_x, position.y);
  vec3 base_yv = vec3_new(position.x, base_y, position.y+1);
  
  return vec3_normalize(vec3_cross(vec3_sub(base_yv, basev), vec3_sub(base_xv, basev)));
  
}
