#include <math.h>

#include "marching_cubes.h"

#include "kernel.h"

static kernel_memory volume;
static kernel_memory point_color_buffer;

static kernel marching_cubes;
static kernel write_clear;
static kernel write_point;
static kernel write_metaball;
static kernel write_point_color_back;

static GLuint point_positions;
static GLuint point_colors;

const int width = 16;
const int height = 16;
const int depth = 16;


#define MAX_VERTS 10000

static GLuint vertex_positions;
static GLuint vertex_normals;
static GLuint vertex_index;

static kernel_memory vertex_positions_buffer;
static kernel_memory vertex_normals_buffer;
static kernel_memory vertex_index_buffer;
static kernel_memory vertex_num_verts

static int num_verts = 0;

void marching_cubes_init() {
  
  const int full_size = width * height * depth;
  
  /* Point rendering data */
  
  vector4* point_data = malloc(sizeof(vector4) * full_size);
  
  if(point_data == NULL) {
    error("Not enough memory!");
  }

  int x, y, z;
  for(x = 0; x < width; x++)
  for(y = 0; y < height; y++)
  for(z = 0; z < depth; z++) {
    int id = x + y * width + z * width * height;
    vector4 position = v4_sub( v4(x, y, z, 1), v4(width/2, height/2, depth/2, 0));
    point_data[id] = position;
  }
  
  glGenBuffers(1, &point_positions);
  glBindBuffer(GL_ARRAY_BUFFER, point_positions);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * full_size, point_data, GL_STATIC_DRAW);
  free(point_data);
  
  vector4* point_color_data = malloc(sizeof(vector4) * full_size);
  memset(point_color_data, 0, sizeof(vector4) * full_size);
  glGenBuffers(1, &point_colors);
  glBindBuffer(GL_ARRAY_BUFFER, point_colors);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * full_size, point_color_data, GL_DYNAMIC_COPY);
  free(point_color_data);
  
  point_color_buffer = kernel_memory_from_glbuffer(point_colors);
  
  /* OpenCL volume */
  
  volume = kernel_memory_allocate(sizeof(float) * full_size);
  
  /* Kernels */
  
  kernel_program* marching_cubes = asset_get("./kernels/marching_cubes.cl");
  
  write_point = kernel_program_get_kernel(marching_cubes, "write_point");
  kernel_set_argument(write_point, 0, sizeof(kernel_memory), &volume);
  kernel_set_argument(write_point, 4, sizeof(int), (void*)&width);
  kernel_set_argument(write_point, 5, sizeof(int), (void*)&height);
  kernel_set_argument(write_point, 6, sizeof(int), (void*)&depth);
  
  write_metaball = kernel_program_get_kernel(marching_cubes, "write_metaball");
  kernel_set_argument(write_metaball, 0, sizeof(kernel_memory), &volume);
  
  write_clear = kernel_program_get_kernel(marching_cubes, "write_clear");
  kernel_set_argument(write_clear, 0, sizeof(kernel_memory), &volume);
  
  write_point_color_back = kernel_program_get_kernel(marching_cubes, "write_point_color_back");
  kernel_set_argument(write_point_color_back, 0, sizeof(kernel_memory), &volume);
  kernel_set_argument(write_point_color_back, 1, sizeof(kernel_memory), &point_color_buffer);
}

void marching_cubes_finish() {
  
  glDeleteBuffers(1, &point_positions);
  glDeleteBuffers(1, &point_colors);
  
  kernel_memory_delete(volume);
  kernel_memory_delete(point_color_buffer);
  
}

void marching_cubes_clear() {
  kernel_run(write_clear, width * height * depth);
}

void marching_cubes_point(int x, int y, int z, float value) {

  if ((x >= width) || (y >= height) || (z >= depth) || (x < 0) || (y < 0) || (z < 0)) {
    error("Point (%i, %i, %i) outside of volume");
  }

  kernel_set_argument(write_point, 1, sizeof(int), &x);
  kernel_set_argument(write_point, 2, sizeof(int), &y);
  kernel_set_argument(write_point, 3, sizeof(int), &z);
  kernel_set_argument(write_point, 7, sizeof(float), &value);
  kernel_run(write_point, 1);
}

void marching_cubes_metaball(float x, float y, float z, float size) {
  
  int bot_x = max(floor(x) - size, 0);
  int bot_y = max(floor(y) - size, 0);
  int bot_z = max(floor(z) - size, 0);
  
  int top_x = min(ceil(x) + size, width-1);
  int top_y = min(ceil(y) + size, height-1);
  int top_z = min(ceil(z) + size, depth-1);
  
  int count = (top_x - bot_x) * (top_y - bot_y) * (top_z - bot_z);
  
  kernel_set_argument(write_metaball, 1, sizeof(int), &bot_x);
  kernel_set_argument(write_metaball, 2, sizeof(int), &bot_y);
  kernel_set_argument(write_metaball, 3, sizeof(int), &bot_z);
  kernel_set_argument(write_metaball, 4, sizeof(int), &top_x);
  kernel_set_argument(write_metaball, 5, sizeof(int), &top_y);
  kernel_set_argument(write_metaball, 6, sizeof(int), &top_z);
  kernel_set_argument(write_metaball, 7, sizeof(int), (void*)&width);
  kernel_set_argument(write_metaball, 8, sizeof(int), (void*)&height);
  kernel_set_argument(write_metaball, 9, sizeof(int), (void*)&depth);
  kernel_set_argument(write_metaball, 10, sizeof(float), &x);
  kernel_set_argument(write_metaball, 11, sizeof(float), &y);
  kernel_set_argument(write_metaball, 12, sizeof(float), &z);
  kernel_set_argument(write_metaball, 13, sizeof(float), &size);
  kernel_run(write_metaball, count);
  
}

void marching_cubes_update() {

}

void marching_cubes_render() {

  const int full_size = width * height * depth;
  
  kernel_memory_gl_aquire(point_color_buffer);
  kernel_run(write_point_color_back, full_size);
  kernel_memory_gl_release(point_color_buffer);
  
  /* Before we do the metaballs lets just do a rendering based upon volume data */
  
  glPointSize(4.0f);
  
  glBindBuffer(GL_ARRAY_BUFFER, point_positions);
  glVertexPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, point_colors);
  glColorPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_COLOR_ARRAY);
  
  glDrawArrays(GL_POINTS, 0, full_size);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glPointSize(1.0f);
}