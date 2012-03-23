#include <math.h>

#include "corange.h"
#include "kernel.h"

#include "volume_renderer.h"

static int WIDTH;
static int HEIGHT;
static int DEPTH;

static int SCREEN_WIDTH;
static int SCREEN_HEIGHT;

static GLuint depth_texture;
static GLuint stencil_texture;
static GLuint positions_texture;
static GLuint normals_texture;

static kernel_memory k_depth_texture;
static kernel_memory k_stencil_texture;
static kernel_memory k_positions_texture;
static kernel_memory k_normals_texture;

static kernel_memory k_volume;

static kernel k_write_point;
static kernel k_write_metaballs;
static kernel k_write_particles;

static kernel k_clear_volume;
static kernel k_clear_texture;

static kernel k_generate_depth;
static kernel k_generate_positions;
static kernel k_generate_normals;

static camera* cam = NULL;
static light* sun = NULL;

static texture* env_map = NULL;
static shader_program* metaballs_def = NULL;

void volume_renderer_set_camera(camera* new_cam) {
  cam = new_cam;
}

void volume_renderer_set_light(light* new_light) {
  sun = new_light;
}

void volume_renderer_init() {
  
  WIDTH = graphics_viewport_width() / 12;
  HEIGHT = graphics_viewport_height() / 12;
  DEPTH = 128;
  
  SCREEN_WIDTH = graphics_viewport_width();
  SCREEN_HEIGHT = graphics_viewport_height();
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA16, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  k_depth_texture = kernel_memory_from_gltexture2D(depth_texture);
  
  glGenTextures(1, &stencil_texture);
  glBindTexture(GL_TEXTURE_2D, stencil_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  k_stencil_texture = kernel_memory_from_gltexture2D(stencil_texture);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  k_positions_texture = kernel_memory_from_gltexture2D(positions_texture);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  k_normals_texture = kernel_memory_from_gltexture2D(normals_texture);
  
  float* volume_data = calloc(WIDTH * HEIGHT * DEPTH, sizeof(float));
  
  k_volume = kernel_memory_allocate(WIDTH * HEIGHT * DEPTH * sizeof(float));
  kernel_memory_write(k_volume, WIDTH * HEIGHT * DEPTH * sizeof(float), volume_data);
  
  free(volume_data);
  
  kernel_program* volume_rendering = asset_get("./kernels/volume_rendering.cl");
  
  k_write_point = kernel_program_get_kernel(volume_rendering, "write_point");
  k_write_metaballs = kernel_program_get_kernel(volume_rendering, "write_metaballs");
  k_write_particles = kernel_program_get_kernel(volume_rendering, "write_particles");
  
  
  k_clear_volume = kernel_program_get_kernel(volume_rendering, "clear_volume");
  k_clear_texture = kernel_program_get_kernel(volume_rendering, "clear_texture");
  
  k_generate_depth = kernel_program_get_kernel(volume_rendering, "generate_depth");
  k_generate_positions = kernel_program_get_kernel(volume_rendering, "generate_positions");
  k_generate_normals = kernel_program_get_kernel(volume_rendering, "generate_normals");
  
  env_map = asset_load_get("./resources/metaballs_env.dds");
  metaballs_def = asset_load_get("./shaders/metaballs_def.prog");
}

void volume_renderer_finish() {
  
  glDeleteTextures(1, &depth_texture);
  glDeleteTextures(1, &stencil_texture);
  glDeleteTextures(1, &positions_texture);
  glDeleteTextures(1, &normals_texture);
  
  kernel_memory_delete(k_depth_texture);
  kernel_memory_delete(k_stencil_texture);
  kernel_memory_delete(k_positions_texture);
  kernel_memory_delete(k_normals_texture);
  
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
  
  vector4 point = v4(sun->position.x, sun->position.y, sun->position.z, 1);
  
    kernel_set_argument(k_clear_volume, 0, sizeof(kernel_memory), &k_volume);
    kernel_set_argument(k_clear_volume, 1, sizeof(cl_int3), &size);
    kernel_run(k_clear_volume, WIDTH * HEIGHT * DEPTH);
  
  int tex_size[2] = {WIDTH, HEIGHT};
  vector4 black = v4_black();
  
  kernel_memory_gl_aquire(k_stencil_texture);
  
    kernel_set_argument(k_clear_texture, 0, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_clear_texture, 1, sizeof(cl_int2), &tex_size);
    kernel_set_argument(k_clear_texture, 2, sizeof(vector4), &black);
    kernel_run(k_clear_texture, WIDTH * HEIGHT);
  
  kernel_memory_gl_release(k_stencil_texture);
  
  kernel_memory_gl_aquire(k_stencil_texture);
  
    kernel_set_argument(k_write_point, 0, sizeof(kernel_memory), &k_volume);
    kernel_set_argument(k_write_point, 1, sizeof(cl_int3), &size);
    kernel_set_argument(k_write_point, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_write_point, 3, sizeof(matrix_4x4), &view_matrix);
    kernel_set_argument(k_write_point, 4, sizeof(matrix_4x4), &proj_matrix);
    kernel_set_argument(k_write_point, 5, sizeof(vector4), &point);
    kernel_run(k_write_point, 1);
  
  kernel_memory_gl_release(k_stencil_texture);
  
  kernel_memory_gl_aquire(k_stencil_texture);
  kernel_memory_gl_aquire(metaball_positions);
  
    kernel_set_argument(k_write_particles, 0, sizeof(kernel_memory), &k_volume);
    kernel_set_argument(k_write_particles, 1, sizeof(cl_int3), &size);
    kernel_set_argument(k_write_particles, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_write_particles, 3, sizeof(matrix_4x4), &view_matrix);
    kernel_set_argument(k_write_particles, 4, sizeof(matrix_4x4), &proj_matrix);
    kernel_set_argument(k_write_particles, 5, sizeof(kernel_memory), &metaball_positions);
    kernel_run(k_write_particles, num_metaballs);
  
  kernel_memory_gl_release(metaball_positions);
  kernel_memory_gl_release(k_stencil_texture);
  
  kernel_memory_gl_aquire(metaball_positions);
  kernel_memory_gl_aquire(k_stencil_texture);
  
    kernel_set_argument(k_write_metaballs, 0, sizeof(kernel_memory), &k_volume);
    kernel_set_argument(k_write_metaballs, 1, sizeof(cl_int3), &size);
    kernel_set_argument(k_write_metaballs, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_write_metaballs, 3, sizeof(matrix_4x4), &inv_view_matrix);
    kernel_set_argument(k_write_metaballs, 4, sizeof(matrix_4x4), &inv_proj_matrix);
    kernel_set_argument(k_write_metaballs, 5, sizeof(kernel_memory), &metaball_positions);
    kernel_set_argument(k_write_metaballs, 6, sizeof(cl_int), &num_metaballs);
    kernel_run(k_write_metaballs, WIDTH * HEIGHT * DEPTH);
  
  kernel_memory_gl_release(k_stencil_texture);
  kernel_memory_gl_release(metaball_positions);
  
}

void volume_renderer_render() {
  
  int size[3] = {WIDTH, HEIGHT, DEPTH};
  
  int screen_size[2] = {SCREEN_WIDTH, SCREEN_HEIGHT};
  
  matrix_4x4 view_matrix = camera_view_matrix(cam);
  matrix_4x4 proj_matrix = camera_proj_matrix(cam, graphics_viewport_ratio());
  
  matrix_4x4 inv_view_matrix = m44_inverse(view_matrix);
  matrix_4x4 inv_proj_matrix = m44_inverse(proj_matrix);
  
  kernel_memory_gl_aquire(k_stencil_texture);
  kernel_memory_gl_aquire(k_depth_texture);
  kernel_memory_gl_aquire(k_positions_texture);
  kernel_memory_gl_aquire(k_normals_texture);
  kernel_memory_gl_aquire(metaball_positions);
  
    kernel_set_argument(k_generate_depth, 0, sizeof(kernel_memory), &k_volume);
    kernel_set_argument(k_generate_depth, 1, sizeof(cl_int3), &size);
    kernel_set_argument(k_generate_depth, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_generate_depth, 3, sizeof(kernel_memory), &k_depth_texture);
    kernel_set_argument(k_generate_depth, 4, sizeof(cl_int2), &screen_size);
    kernel_run(k_generate_depth, SCREEN_WIDTH * SCREEN_HEIGHT);

    kernel_set_argument(k_generate_positions, 0, sizeof(kernel_memory), &k_depth_texture);
    kernel_set_argument(k_generate_positions, 1, sizeof(kernel_memory), &k_positions_texture);
    kernel_set_argument(k_generate_positions, 2, sizeof(matrix_4x4), &inv_view_matrix);
    kernel_set_argument(k_generate_positions, 3, sizeof(matrix_4x4), &inv_proj_matrix);
    kernel_set_argument(k_generate_positions, 4, sizeof(cl_int2), &screen_size);
    kernel_run(k_generate_positions, SCREEN_WIDTH * SCREEN_HEIGHT);

    kernel_set_argument(k_generate_normals, 0, sizeof(kernel_memory), &k_depth_texture);
    kernel_set_argument(k_generate_normals, 1, sizeof(kernel_memory), &k_positions_texture);
    kernel_set_argument(k_generate_normals, 2, sizeof(kernel_memory), &k_normals_texture);
    kernel_set_argument(k_generate_normals, 3, sizeof(cl_int2), &screen_size);
    kernel_set_argument(k_generate_normals, 4, sizeof(kernel_memory), &metaball_positions);
    kernel_set_argument(k_generate_normals, 5, sizeof(cl_int), &num_metaballs);
    kernel_run(k_generate_normals, SCREEN_WIDTH * SCREEN_HEIGHT);
  
  kernel_memory_gl_release(metaball_positions);
  kernel_memory_gl_release(k_stencil_texture);
  kernel_memory_gl_release(k_depth_texture);
  kernel_memory_gl_release(k_positions_texture);
  kernel_memory_gl_release(k_normals_texture);
  
  GLuint handle = shader_program_handle(metaballs_def);
  glUseProgram(handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  GLint camera_position = glGetUniformLocation(handle, "camera_position");
  glUniform3f(camera_position, cam->position.x, cam->position.y, cam->position.z);
  
  GLint light_position = glGetUniformLocation(handle, "light_position");
  glUniform3f(light_position, sun->position.x, sun->position.y, sun->position.z);
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, stencil_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(handle, "stencil_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(handle, "depth_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(handle, "positions_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(handle, "normals_texture"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_2D, texture_handle(env_map));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(handle, "env_texture"), 4);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
}
