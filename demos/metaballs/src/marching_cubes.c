#include <math.h>

#include "marching_cubes.h"

#include "kernel.h"

static kernel_memory volume;
static kernel_memory point_color_buffer;

static kernel construct_surface;
static kernel generate_normals;
static kernel generate_normals_smooth;

static kernel write_clear;
static kernel write_point;
static kernel write_metaball;
static kernel write_metaballs;
static kernel write_point_color_back;

static GLuint point_positions;
static GLuint point_colors;

const int width = 64;
const int height = 64;
const int depth = 64;

#define MAX_VERTS 100000

static GLuint vertex_positions;
static GLuint vertex_normals;

static kernel_memory vertex_positions_buffer;
static kernel_memory vertex_normals_buffer;
static kernel_memory vertex_index;

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
    vector4 position = v4(x, y, z, 1);
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
  
  /* Vertex stuff */
  
  vector4* vertex_pos_data = malloc(sizeof(vector4) * MAX_VERTS);
  memset(vertex_pos_data, 0, sizeof(vector4) * MAX_VERTS);
  glGenBuffers(1, &vertex_positions);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * MAX_VERTS, vertex_pos_data, GL_DYNAMIC_COPY);
  free(vertex_pos_data);
  
  vertex_positions_buffer = kernel_memory_from_glbuffer(vertex_positions);
  
  vector4* vertex_norm_data = malloc(sizeof(vector4) * MAX_VERTS);
  memset(vertex_norm_data, 0, sizeof(vector4) * MAX_VERTS);
  glGenBuffers(1, &vertex_normals);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * MAX_VERTS, vertex_norm_data, GL_DYNAMIC_COPY);
  free(vertex_norm_data);
  
  vertex_normals_buffer = kernel_memory_from_glbuffer(vertex_normals);
  
  vertex_index = kernel_memory_allocate(sizeof(int));
  
  /* Kernels */
  
  kernel_program* marching_cubes = asset_get("./kernels/marching_cubes.cl");
  
  write_point = kernel_program_get_kernel(marching_cubes, "write_point");
  kernel_set_argument(write_point, 0, sizeof(kernel_memory), &volume);
  kernel_set_argument(write_point, 4, sizeof(int), (void*)&width);
  kernel_set_argument(write_point, 5, sizeof(int), (void*)&height);
  kernel_set_argument(write_point, 6, sizeof(int), (void*)&depth);
  
  write_metaball = kernel_program_get_kernel(marching_cubes, "write_metaball");
  kernel_set_argument(write_metaball, 0, sizeof(kernel_memory), &volume);
  
  write_metaballs = kernel_program_get_kernel(marching_cubes, "write_metaballs");
  kernel_set_argument(write_metaballs, 0, sizeof(kernel_memory), &volume);
  
  write_clear = kernel_program_get_kernel(marching_cubes, "write_clear");
  kernel_set_argument(write_clear, 0, sizeof(kernel_memory), &volume);
  
  write_point_color_back = kernel_program_get_kernel(marching_cubes, "write_point_color_back");
  kernel_set_argument(write_point_color_back, 0, sizeof(kernel_memory), &volume);
  kernel_set_argument(write_point_color_back, 1, sizeof(kernel_memory), &point_color_buffer);
  
  construct_surface = kernel_program_get_kernel(marching_cubes, "construct_surface");
  kernel_set_argument(construct_surface, 0, sizeof(kernel_memory), &volume);
  
  generate_normals = kernel_program_get_kernel(marching_cubes, "generate_flat_normals");
  
  generate_normals_smooth = kernel_program_get_kernel(marching_cubes, "generate_smooth_normals");
  
}

void marching_cubes_finish() {
  
  glDeleteBuffers(1, &point_positions);
  glDeleteBuffers(1, &point_colors);
    
  kernel_memory_delete(volume);
  kernel_memory_delete(point_color_buffer);
  
  glDeleteBuffers(1, &vertex_positions);
  glDeleteBuffers(1, &vertex_normals);

  kernel_memory_delete(vertex_positions_buffer);
  kernel_memory_delete(vertex_normals_buffer);
  kernel_memory_delete(vertex_index);
  
}

void marching_cubes_clear() {
  kernel_run(write_clear, width * height * depth);
}

void marching_cubes_point(int x, int y, int z, float value) {

  if ((x >= width) || (y >= height) || (z >= depth) || (x < 0) || (y < 0) || (z < 0)) {
    error("Point (%i, %i, %i) outside of volume", x, y, z);
  }

  kernel_set_argument(write_point, 1, sizeof(int), &x);
  kernel_set_argument(write_point, 2, sizeof(int), &y);
  kernel_set_argument(write_point, 3, sizeof(int), &z);
  kernel_set_argument(write_point, 7, sizeof(float), &value);
  kernel_run(write_point, 1);
}

/*
void marching_cubes_metaball(float x, float y, float z) {
  
  const int METABALL_SIZE = 10;
  
  int bot_x = max(floor(x) - METABALL_SIZE, 0);
  int bot_y = max(floor(y) - METABALL_SIZE, 0);
  int bot_z = max(floor(z) - METABALL_SIZE, 0);
  
  int top_x = min(ceil(x) + METABALL_SIZE, width-1);
  int top_y = min(ceil(y) + METABALL_SIZE, height-1);
  int top_z = min(ceil(z) + METABALL_SIZE, depth-1);
  
  int count = (top_x - bot_x) * (top_y - bot_y) * (top_z - bot_z);
  
  int bottom[3] = {bot_x, bot_y, bot_z};
  int top[3] = {top_x, top_y, top_z};
  int size[3] = {width, height, depth};
  
  kernel_set_argument(write_metaball, 1, sizeof(cl_int3), bottom);
  kernel_set_argument(write_metaball, 2, sizeof(cl_int3), top);
  kernel_set_argument(write_metaball, 3, sizeof(cl_int3), size);
  kernel_set_argument(write_metaball, 4, sizeof(float), &x);
  kernel_set_argument(write_metaball, 5, sizeof(float), &y);
  kernel_set_argument(write_metaball, 6, sizeof(float), &z);
  kernel_run(write_metaball, count);
  
}
*/

static kernel_memory metaball_positions;
static int num_metaballs;

void marching_cubes_metaball_data(kernel_memory positions, int num_balls) {
  metaball_positions = positions;
  num_metaballs = num_balls;
}

void marching_cubes_update() {

  int size[3] = {width, height, depth};
  
  /* Update volumes */
  
  kernel_memory_gl_aquire(metaball_positions);
  kernel_memory_gl_aquire(vertex_positions_buffer);
  kernel_memory_gl_aquire(vertex_normals_buffer);
  
  kernel_set_argument(write_metaballs, 1, sizeof(cl_int3), &size);
  kernel_set_argument(write_metaballs, 2, sizeof(kernel_memory), &metaball_positions);
  kernel_set_argument(write_metaballs, 3, sizeof(cl_int), &num_metaballs);
  kernel_run(write_metaballs, width * height * depth);
  
  /* End */
  
  int zero = 0;
  kernel_memory_write(vertex_index, sizeof(int), &zero);
  
  const int num_workers = (width-1) * (height-1) * (depth-1);
  
  kernel_set_argument(construct_surface, 0, sizeof(kernel_memory), &volume);
  kernel_set_argument(construct_surface, 1, sizeof(cl_int3), &size);
  kernel_set_argument(construct_surface, 2, sizeof(kernel_memory), &vertex_positions_buffer);
  kernel_set_argument(construct_surface, 3, sizeof(kernel_memory), &vertex_index);
  kernel_run(construct_surface, num_workers);
  
  kernel_memory_read(vertex_index, sizeof(cl_int), &num_verts);
  
  /* Generate Normals */
  
  if (num_verts > 0) {
    kernel_set_argument(generate_normals_smooth, 0, sizeof(kernel_memory), &vertex_positions_buffer);
    kernel_set_argument(generate_normals_smooth, 1, sizeof(kernel_memory), &vertex_normals_buffer);
    kernel_set_argument(generate_normals_smooth, 2, sizeof(kernel_memory), &metaball_positions);
    kernel_set_argument(generate_normals_smooth, 3, sizeof(cl_int), &num_metaballs);
    kernel_run(generate_normals_smooth, num_verts);
  }
  
  /*
  kernel_set_argument(generate_normals, 0, sizeof(kernel_memory), &vertex_positions_buffer);
  kernel_set_argument(generate_normals, 1, sizeof(kernel_memory), &vertex_normals_buffer);
  kernel_run(generate_normals, num_verts/3);
  */
  
  kernel_memory_gl_release(vertex_positions_buffer);
  kernel_memory_gl_release(vertex_normals_buffer);
  kernel_memory_gl_release(metaball_positions);
  
  kernel_run_finish();
  
}

void marching_cubes_render(bool wireframe, camera* c, light* l) {
  
  const int full_size = width * height * depth;
  
  kernel_memory_gl_aquire(point_color_buffer);
  kernel_run(write_point_color_back, full_size);
  kernel_memory_gl_release(point_color_buffer);
  
  /* Before we do the metaballs lets just do a rendering based upon volume data */
  
  /*
  
  glPointSize(1.0f);
  
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
  
  */
  
  /* Then Draw Triangles */
  if (wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  
  shader_program* metaballs = asset_load_get("./shaders/metaballs.prog");
  GLuint NORMALS = glGetAttribLocation(*metaballs, "normals");
  
  glUseProgram(*metaballs);
  
  GLint light_position_u = glGetUniformLocation(*metaballs, "light_position");
  glUniform3f(light_position_u, l->position.x, l->position.y, l->position.z);
  
  GLint camera_position_u = glGetUniformLocation(*metaballs, "camera_position");
  glUniform3f(camera_position_u, c->position.x, c->position.y, c->position.z);
  
  float lproj_matrix[16]; m44_to_array(light_view_matrix(l), lproj_matrix);
  float lview_matrix[16]; m44_to_array(light_proj_matrix(l), lview_matrix);
  
  GLint lproj_matrix_u = glGetUniformLocation(*metaballs, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, lproj_matrix);
  
  GLint lview_matrix_u = glGetUniformLocation(*metaballs, "light_view");
  glUniformMatrix4fv(lview_matrix_u, 1, 0, lview_matrix);
  
  texture* env_map = asset_load_get("./resources/metaballs_env.dds");
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, *env_map);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*metaballs, "env_map"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, *shadow_mapper_depth_texture());
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*metaballs, "shadow_map"), 1);
  
  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions);
  glVertexPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, vertex_normals);
  glVertexAttribPointer(NORMALS, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(NORMALS);
  
  glDrawArrays(GL_TRIANGLES, 0, num_verts);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableVertexAttribArray(NORMALS);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glUseProgram(0);
  
  if (wireframe) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  
}

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];

void marching_cubes_render_shadows(light* l) {
  
  matrix_4x4 viewm = light_view_matrix(l);
  matrix_4x4 projm = light_proj_matrix(l);
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);    
  
  m44_to_array(m44_id(), world_matrix);
  
  shader_program* depth_shader = asset_load_get("$SHADERS/depth.prog");
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions);
  glVertexPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
    glDrawArrays(GL_TRIANGLES, 0, num_verts);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glUseProgram(0);

}
