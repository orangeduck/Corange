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
static kernel k_write_particles;
static kernel k_clear_volume;

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
  DEPTH = 256;
  
  int screen_width = graphics_viewport_width();
  int screen_height = graphics_viewport_height();
  
  char* screen_texture = calloc(screen_width * screen_height, 4);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  k_depth_texture = kernel_memory_from_gltexture2D(depth_texture);
  free(screen_texture);
  
  float* volume_data = calloc(WIDTH * HEIGHT * DEPTH, sizeof(float));
  
  k_volume = kernel_memory_allocate(WIDTH * HEIGHT * DEPTH * sizeof(float));
  kernel_memory_write(k_volume, WIDTH * HEIGHT * DEPTH * sizeof(float), volume_data);
  
  free(volume_data);
  
  kernel_program* volume_rendering = asset_get("./kernels/volume_rendering.cl");
  k_write_point = kernel_program_get_kernel(volume_rendering, "write_point");
  k_write_metaballs = kernel_program_get_kernel(volume_rendering, "write_metaballs");
  k_write_particles = kernel_program_get_kernel(volume_rendering, "write_particles");
  k_trace_ray = kernel_program_get_kernel(volume_rendering, "trace_ray");
  k_clear_volume = kernel_program_get_kernel(volume_rendering, "clear_volume");
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
  
  kernel_set_argument(k_clear_volume, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_clear_volume, 1, sizeof(cl_int3), &size);
  kernel_run(k_clear_volume, WIDTH * HEIGHT * DEPTH);
  
  kernel_set_argument(k_write_point, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_write_point, 1, sizeof(cl_int3), &size);
  kernel_set_argument(k_write_point, 2, sizeof(matrix_4x4), &view_matrix);
  kernel_set_argument(k_write_point, 3, sizeof(matrix_4x4), &proj_matrix);
  
  vector4 point;
  
  point = v4(0,0,0,1);
  //kernel_set_argument(k_write_point, 4, sizeof(vector4), &point);
  //kernel_run(k_write_point, 1);
  
  //vector3 clip_space = v3(point.x, point.y, point.z);
  //clip_space = m44_mul_v3(view_matrix, clip_space);
  //clip_space = m44_mul_v3(proj_matrix, clip_space);
  
  //clip_space.z = pow(clip_space.z, 100);
  
  //debug("Point: (%0.2f, %0.2f, %f)", clip_space.x, clip_space.y, clip_space.z);
  
  point = v4(sun->position.x, sun->position.y, sun->position.z, 1);
  kernel_set_argument(k_write_point, 4, sizeof(vector4), &point);
  kernel_run(k_write_point, 1);
  
  kernel_memory_gl_aquire(metaball_positions);
  
  kernel_set_argument(k_write_particles, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_write_particles, 1, sizeof(cl_int3), &size);
  kernel_set_argument(k_write_particles, 2, sizeof(matrix_4x4), &view_matrix);
  kernel_set_argument(k_write_particles, 3, sizeof(matrix_4x4), &proj_matrix);
  kernel_set_argument(k_write_particles, 4, sizeof(kernel_memory), &metaball_positions);
  kernel_run(k_write_particles, num_metaballs);
  
  kernel_memory_gl_release(metaball_positions);
  
  vector3 clip_point = v3(WIDTH/2, HEIGHT/2, -509);
  clip_point.x = clip_point.x / (WIDTH-1);
  clip_point.y = clip_point.y / (HEIGHT-1);
  clip_point.z = clip_point.z / (DEPTH-1);
  
  clip_point.x = (clip_point.x * 2) - 1;
  clip_point.y = (clip_point.y * 2) - 1;
  //clip_point.z = pow(clip_point.z, (1/100));
  
  vector3 world_point = clip_point;
  world_point = m44_mul_v3(inv_proj_matrix, world_point);
  world_point = m44_mul_v3(inv_view_matrix, world_point);
  
  //debug("World Point: (%0.2f, %0.2f, %0.2f)", world_point.x, world_point.y, world_point.z);
  
  kernel_memory_gl_aquire(metaball_positions);
  
  kernel_set_argument(k_write_metaballs, 0, sizeof(kernel_memory), &k_volume);
  kernel_set_argument(k_write_metaballs, 1, sizeof(cl_int3), &size);
  kernel_set_argument(k_write_metaballs, 2, sizeof(matrix_4x4), &inv_view_matrix);
  kernel_set_argument(k_write_metaballs, 3, sizeof(matrix_4x4), &inv_proj_matrix);
  kernel_set_argument(k_write_metaballs, 4, sizeof(kernel_memory), &metaball_positions);
  kernel_set_argument(k_write_metaballs, 5, sizeof(cl_int), &num_metaballs);
  kernel_run(k_write_metaballs, WIDTH * HEIGHT * DEPTH);
  
  kernel_memory_gl_release(metaball_positions);
  
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
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}
