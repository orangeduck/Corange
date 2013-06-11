#include "assets/terrain.h"

#include "cnet.h"

#include "assets/cmesh.h"

void terrain_chunk_delete(terrain_chunk* tc) {
  
  if (net_is_client()) {
    glDeleteBuffers(NUM_TERRAIN_BUFFERS, tc->index_buffers);
    glDeleteBuffers(1, &tc->vertex_buffer);
  }
  
  cmesh_delete(tc->colmesh);
  
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
  
  int x_max = tc->width * SUBDIVISIONS + 1;
  int y_max = tc->height * SUBDIVISIONS + 1;
  
  tc->num_verts = x_max * y_max + x_max * 2 + y_max * 2;
  vec3* vertex_buffer = malloc(sizeof(vec3) * 4 * tc->num_verts);
  int index = 0;
  
  for(int x = 0; x < x_max; x++)
  for(int y = 0; y < y_max; y++) {
    float gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    float gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float height = terrain_height(ter, vec2_new(gx, gy));
    mat3  axis   = terrain_tbn(ter, vec2_new(gx, gy));
    
    vec3 pos = vec3_new(gx, height, gy);
    vec3 tangent = mat3_mul_vec3(axis, vec3_new(1,0,0));
    vec3 normal  = mat3_mul_vec3(axis, vec3_new(0,1,0));
    vec3 binorm  = mat3_mul_vec3(axis, vec3_new(0,0,1));
    
    vertex_buffer[index] = pos; index++;
    vertex_buffer[index] = normal; index++;
    vertex_buffer[index] = tangent; index++;
    vertex_buffer[index] = binorm; index++;
    
  }
  
  /* Adding fins. Don't look, horrible code */
  
  const float FIN_DEPTH = 5.0;
  
  for(int y = 0; y < y_max; y++) {
    int gx = tc->x * ter->chunk_width + 0;
    int gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float height = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    mat3  axis   = terrain_tbn(ter, vec2_new(gx, gy));
    
    vec3 pos = vec3_new(gx, height, gy);
    vec3 tangent = mat3_mul_vec3(axis, vec3_new(1,0,0));
    vec3 normal  = mat3_mul_vec3(axis, vec3_new(0,1,0));
    vec3 binorm  = mat3_mul_vec3(axis, vec3_new(0,0,1));
    
    vertex_buffer[index] = pos; index++;
    vertex_buffer[index] = normal; index++;
    vertex_buffer[index] = tangent; index++;
    vertex_buffer[index] = binorm; index++;
  }
  
  for(int y = 0; y < y_max; y++) {
    int gx = tc->x * ter->chunk_width + ter->chunk_width;
    int gy = tc->y * ter->chunk_height + (float)y/SUBDIVISIONS;
    
    float height = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    mat3  axis   = terrain_tbn(ter, vec2_new(gx, gy));
    
    vec3 pos = vec3_new(gx, height, gy);
    vec3 tangent = mat3_mul_vec3(axis, vec3_new(1,0,0));
    vec3 normal  = mat3_mul_vec3(axis, vec3_new(0,1,0));
    vec3 binorm  = mat3_mul_vec3(axis, vec3_new(0,0,1));
    
    vertex_buffer[index] = pos; index++;
    vertex_buffer[index] = normal; index++;
    vertex_buffer[index] = tangent; index++;
    vertex_buffer[index] = binorm; index++;
  }
  
  for(int x = 0; x < x_max; x++) {
    int  gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    int  gy = tc->y * ter->chunk_height + 0;
    
    float height = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    mat3  axis   = terrain_tbn(ter, vec2_new(gx, gy));
    
    vec3 pos = vec3_new(gx, height, gy);
    vec3 tangent = mat3_mul_vec3(axis, vec3_new(1,0,0));
    vec3 normal  = mat3_mul_vec3(axis, vec3_new(0,1,0));
    vec3 binorm  = mat3_mul_vec3(axis, vec3_new(0,0,1));
    
    vertex_buffer[index] = pos; index++;
    vertex_buffer[index] = normal; index++;
    vertex_buffer[index] = tangent; index++;
    vertex_buffer[index] = binorm; index++;
  }
  
  for(int x = 0; x < x_max; x++) {
    int  gx = tc->x * ter->chunk_width + (float)x/SUBDIVISIONS;
    int  gy = tc->y * ter->chunk_height + ter->chunk_height;
    
    float height = terrain_height(ter, vec2_new(gx, gy)) - FIN_DEPTH;
    mat3  axis   = terrain_tbn(ter, vec2_new(gx, gy));
    
    vec3 pos = vec3_new(gx, height, gy);
    vec3 tangent = mat3_mul_vec3(axis, vec3_new(1,0,0));
    vec3 normal  = mat3_mul_vec3(axis, vec3_new(0,1,0));
    vec3 binorm  = mat3_mul_vec3(axis, vec3_new(0,0,1));
    
    vertex_buffer[index] = pos; index++;
    vertex_buffer[index] = normal; index++;
    vertex_buffer[index] = tangent; index++;
    vertex_buffer[index] = binorm; index++;
  }
  
  tc->bound.center = vec3_zero();
  for (int i = 0; i < index; i+=4) {
    tc->bound.center = vec3_add(tc->bound.center, vertex_buffer[i]);
  }
  tc->bound.center = vec3_div(tc->bound.center, tc->num_verts);
  
  tc->bound.radius = 0;
  for (int i = 0; i < index; i+=4) {
    tc->bound.radius = max(tc->bound.radius, vec3_dist(tc->bound.center, vertex_buffer[i]));
  }
  
  if (net_is_client()) {
    glGenBuffers(1, &tc->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 4 * tc->num_verts, vertex_buffer, GL_STATIC_DRAW);
  }
  
  free(vertex_buffer);
  
  if (net_is_client()) {
    glGenBuffers(NUM_TERRAIN_BUFFERS, tc->index_buffers);
  }
  
  for(int j = 0; j < NUM_TERRAIN_BUFFERS; j++) {
  
    int off = pow(2, j);
    int x_max = tc->width * SUBDIVISIONS;
    int y_max = tc->height * SUBDIVISIONS;
    
    tc->num_indicies[j] = (x_max / off) * (y_max / off) * 6 + (x_max / off) * 12 + (y_max / off) * 12;
    
    uint32_t* index_buffer = malloc(sizeof(uint32_t) * tc->num_indicies[j]);
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
    
    if (net_is_client()) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[j]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * tc->num_indicies[j], index_buffer, GL_DYNAMIC_DRAW);
    }
    
    free(index_buffer);
  }
  
  tc->colmesh = malloc(sizeof(cmesh));
  tc->colmesh->is_leaf = true;
  tc->colmesh->triangles_num = (tc->width/4) * (tc->height/4) * 2;
  tc->colmesh->triangles = malloc(sizeof(ctri) * tc->colmesh->triangles_num);
  
  int tri_i = 0;
  
  for (int x = 0; x < tc->width;  x += 4)
  for (int y = 0; y < tc->height; y += 4) {
    
    float gx = tc->x * ter->chunk_width  + (float)x;
    float gy = tc->y * ter->chunk_height + (float)y;    
    
    vec3 a = vec3_new(gx  , terrain_height(ter, vec2_new(gx  , gy  )) , gy  );
    vec3 b = vec3_new(gx+4, terrain_height(ter, vec2_new(gx+4, gy  )) , gy  );
    vec3 c = vec3_new(gx+4, terrain_height(ter, vec2_new(gx+4, gy+4)) , gy+4);
    vec3 d = vec3_new(gx  , terrain_height(ter, vec2_new(gx  , gy+4)) , gy+4);
    
    vec3 tang   = vec3_normalize(vec3_sub(b, a));
    vec3 binorm = vec3_normalize(vec3_sub(d, a));
    vec3 norm   = vec3_cross( binorm, tang );
    
    tc->colmesh->triangles[tri_i] = ctri_new(a, c, b, norm); tri_i++;
    tc->colmesh->triangles[tri_i] = ctri_new(a, d, c, norm); tri_i++;
  
  }
  
  tc->colmesh->bound = cmesh_bound(tc->colmesh);
  
  /* For some reason this is not working correctly */
  //cmesh_subdivide(tc->colmesh, 5);

  ter->chunks[i] = tc;

}

void terrain_reload_chunk(terrain* ter, int i) {

  terrain_chunk_delete(ter->chunks[i]);
  terrain_new_chunk(ter, i);

  for(int i = 0; i < ter->num_chunks; i++) {
    int x = i % ter->num_cols;
    int y = i / ter->num_cols;
    
    ter->chunks[i]->left   = terrain_get_chunk(ter, x-1, y);
    ter->chunks[i]->right  = terrain_get_chunk(ter, x+1, y);
    ter->chunks[i]->top    = terrain_get_chunk(ter, x, y-1);
    ter->chunks[i]->bottom = terrain_get_chunk(ter, x, y+1);
  }

}

static const float MAX_HEIGHT = 128;

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
  
  const int CHUNK_SIZE = 64;
  
  terrain* ter = malloc(sizeof(terrain));
  ter->width = width;
  ter->height = height;
  ter->chunk_width = CHUNK_SIZE;
  ter->chunk_height = CHUNK_SIZE;
  ter->num_cols = (ter->width / ter->chunk_width);
  ter->num_rows = (ter->height / ter->chunk_height);
  ter->num_chunks = ter->num_cols * ter->num_rows;
  ter->heightmap = malloc(sizeof(float) * width * height);
  
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

void raw_save_file(terrain* t, char* filename) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "wb");
  
  if (!file) {
    error("Could not load file %s\n", filename);
  }
  
  uint16_t* pixels = malloc(sizeof(uint16_t) * t->width * t->height);
  
  for(int i = 0; i < t->width * t->height; i++) {
    pixels[i] = t->heightmap[i] * (65536.0 / MAX_HEIGHT);
  }
  
  SDL_RWwrite(file, pixels, sizeof(uint16_t) * t->width * t->height, 1);
  
  SDL_RWclose(file);
  
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

mat3 terrain_tbn(terrain* ter, vec2 position) {

  float offset   = terrain_height(ter, position);
  float offset_x = terrain_height(ter, vec2_add(position, vec2_new(1,0)));
  float offset_y = terrain_height(ter, vec2_add(position, vec2_new(0,1)));
  
  vec3 pos    = vec3_new(position.x+0, offset,   position.y+0);
  vec3 pos_xv = vec3_new(position.x+1, offset_x, position.y+0);
  vec3 pos_yv = vec3_new(position.x+0, offset_y, position.y+1);
  
  vec3 tangent = vec3_normalize(vec3_sub(pos, pos_xv));
  vec3 binorm  = vec3_normalize(vec3_sub(pos, pos_yv));
  vec3 normal  = vec3_cross(binorm, tangent);

  return mat3_new(
    tangent.x, tangent.y, tangent.z,
    normal.x,  normal.y,  normal.z, 
    binorm.x,  binorm.y,  binorm.z);

}

mat3 terrain_axis(terrain* ter, vec2 position) {

  float offset   = terrain_height(ter, position);
  float offset_x = terrain_height(ter, vec2_add(position, vec2_new(1,0)));
  float offset_y = terrain_height(ter, vec2_add(position, vec2_new(0,1)));
  
  vec3 pos    = vec3_new(position.x+0, offset,   position.y+0);
  vec3 pos_xv = vec3_new(position.x+1, offset_x, position.y+0);
  vec3 pos_yv = vec3_new(position.x+0, offset_y, position.y+1);
  
  vec3 tangent = vec3_normalize(vec3_sub(pos_xv, pos));
  vec3 binorm  = vec3_normalize(vec3_sub(pos_yv, pos));
  vec3 normal  = vec3_cross(binorm, tangent);

  return mat3_new(
    tangent.x, tangent.y, tangent.z,
    normal.x,  normal.y,  normal.z, 
    binorm.x,  binorm.y,  binorm.z);
  
}

vec3 terrain_normal(terrain* ter, vec2 position) {
  
  mat3 axis = terrain_axis(ter, position);
  
  return mat3_mul_vec3(axis, vec3_new(0, 1, 0));
  
}
