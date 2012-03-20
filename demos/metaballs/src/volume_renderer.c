#include <math.h>

#include "corange.h"
#include "kernel.h"

#include "volume_renderer.h"

static int WIDTH;
static int HEIGHT;
static int DEPTH;

static GLuint depth_texture;
static kernel_memory k_depth_texture;

static kernel_memory k_volume;

static kernel k_write_point;
static kernel k_write_metaballs;

static kernel k_trace_ray;

static camera* cam = NULL;
static light* sun = NULL;

void volume_renderer_set_camera(camera* new_cam) {
  cam = new_cam;
}

void volume_renderer_set_light(light* new_light) {
  sun = new_light;
}

void volume_renderer_init() {
  
  WIDTH = graphics_viewport_width() / 4;
  HEIGHT = graphics_viewport_height() / 4;
  DEPTH = 512;
  
  int screen_width = graphics_viewport_width();
  int screen_height = graphics_viewport_height();
  
  char* screen_texture = calloc(screen_width * screen_height, 4);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width,screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen_texture);
  
  k_depth_texture = kernel_memory_from_gltexture2D(depth_texture);
  
  free(screen_texture);
  
  float* volume_data = calloc(WIDTH * HEIGHT * DEPTH, sizeof(float));
  
  k_volume = kernel_memory_allocate(WIDTH * HEIGHT * DEPTH * sizeof(float));
  kernel_memory_write(k_volume, WIDTH * HEIGHT * DEPTH * sizeof(float), volume_data);
  
  free(volume_data);
  
  kernel_program* volume_rendering = asset_get("./kernels/volume_rendering.cl");
  k_write_point = kernel_program_get_kernel(volume_rendering, "write_point");
  k_write_metaballs = kernel_program_get_kernel(volume_rendering, "write_metaballs");
  
}

void volume_renderer_finish() {
  glDeleteTextures(1, &depth_texture);
  kernel_memory_delete(k_depth_texture);
  
  kernel_memory_delete(k_volume);
}

static kernel_memory metaball_positions;
static int num_metaballs;

void volume_renderer_metaball_data(kernel_memory positions, int num_balls) {
  metaball_positions = positions;
  num_metaballs = num_balls;
}

void volume_renderer_update() {
  
  int size[3] = {WIDTH, HEIGHT, DEPTH};
  
  matrix_4x4 view_matrix = camera_view_matrix(cam);
  matrix_4x4 proj_matrix = camera_proj_matrix(cam, graphics_viewport_ratio());
  
  matrix_4x4 inv_view_matrix = m44_inverse(view_matrix);
  matrix_4x4 inv_proj_matrix = m44_inverse(proj_matrix);
  
  kernel_set_argument(k_write_metaballs, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_write_metaballs, 1, sizeof(cl_int3), &size);
  kernel_set_argument(k_write_metaballs, 2, sizeof(matrix_4x4), &inv_view_matrix);
  kernel_set_argument(k_write_metaballs, 3, sizeof(matrix_4x4), &inv_proj_matrix);
  kernel_set_argument(k_write_metaballs, 4, sizeof(kernel_memory), &metaball_positions);
  kernel_set_argument(k_write_metaballs, 5, sizeof(cl_int), &num_metaballs);
  kernel_run(k_write_metaballs, WIDTH * HEIGHT * DEPTH);
}

void volume_renderer_render() {
  
  int size[3] = {WIDTH, HEIGHT, DEPTH};
  
  int screen_width = graphics_viewport_width();
  int screen_height = graphics_viewport_height();
  
  int screen_size[2] = {screen_width, screen_height};
  
  kernel_memory_gl_aquire(k_depth_texture);
  
  kernel_set_argument(k_trace_ray, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_trace_ray, 1, sizeof(cl_int3), &size);
  kernel_set_argument(k_trace_ray, 2, sizeof(kernel_memory), &k_depth_texture);
  kernel_set_argument(k_trace_ray, 3, sizeof(cl_int2), &screen_size);
  kernel_run(k_trace_ray, screen_width * screen_height);
  
  kernel_memory_gl_release(k_depth_texture);
  
}
