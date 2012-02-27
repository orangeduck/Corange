#include <math.h>

#include "corange.h"
#include "kernel.h"

#include "volume_renderer.h"

/* Okay lets see how this is going to work.

  First, we have a volume in clip space.
    - To render this to the screen we must "flatten" it.
    - To do this we just step along the z dimension until we reach a "valid" voxel. We then return the color for this voxel on the x,y.
    - Easy but slow.
    
  How do we blit to this clip space volume?
    - Our Functions for metaballs are in world space. Will be distorted if we try to run them in Clip Space
    - For each voxel we look at in the clip space volume.
      * Convert to world space using inverse view proj matrix.
      * Run through metaball density function.
      * We have our value.
      
   All this is slow. How do we speed up.
    - To render a metaball
      * calculate a bounding box for the metaball in world space
      * convert this into clip space using viewproj matrix on each corner
      * Convert this into an axis aligned bounding box in clip space (Take the bounding box of this distorted bounding box)
      * For each of the voxels in this clip-space section -
        + Convert to world space and run through metaball function.
        + Add intensity to existing intensity.
        + mix color with existing (normal or other properties too).
        + mark clip-space x,y in stencil buffer.
    
    - To flatten the volume
      * We have used the stencil buffer to mark where there potentially exists a volume
      * For each x,y pixel check the stencil buffer is active. If not discard.
      * If active then walk along z axis until above threshold
      * If above threshold then draw color
      * If never above threshold then discard


*/

/*
  So as it turns out you can't write to a 3d texture in openCL. At least on nvidea cards. Bugger.
  
  We have a couple of options:
  
    - We can write to some CPU memory which we can then bind back to some OpenGL memory. This means we have shared memory across the CPU and the GPU twice (once for cl, once for gl).
  
    - Alternative is we set up a framebuffer and render-to-texture on the 3d volume using a pixel shader. Seems better. Though still not ideal.

    I think I may have found a solution.
    
    Create a 3d volume in openGL.
    Create a image object from this in openCL.
    
    Create a buffer in openCL.
      Perform data manipulations on the openCL buffer.
      Use clEnqueueCopyBufferToImage to copy the buffer to the openCL image object.
      Done!
    
*/

int width;
int height;
int depth;

float threshold = 0.75;

GLuint proj_texture;
GLuint stencil_texture;

kernel_memory k_proj_texture;
kernel_memory k_stencil_texture;

kernel_memory k_color_volume;
kernel_memory k_normals_volume;

kernel k_flatten;
kernel k_clear_texture;
kernel k_clear_volume;
kernel k_blit_point;
kernel k_blit_metaball;

camera* cam = NULL;
light* sun = NULL;

void volume_renderer_set_camera(camera* new_cam) {
  cam = new_cam;
}

void volume_renderer_set_light(light* new_light) {
  sun = new_light;
}

void volume_renderer_init() {
  
  width = viewport_width() / 2;
  height = viewport_height() / 2;
  depth = 512;
  
  char* blank_byte_volume = calloc(width * height * depth * 4, sizeof(char));
  char* blank_byte_texture = calloc(width * height * 4, sizeof(char));
  char* blank_stencil_texture = calloc(width * height, sizeof(char));  
  
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &proj_texture);
  glBindTexture(GL_TEXTURE_2D, proj_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_texture);
  
  glGenTextures(1, &stencil_texture);
  glBindTexture(GL_TEXTURE_2D, stencil_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, blank_stencil_texture);
  
  k_proj_texture = kernel_memory_from_gltexture2D(proj_texture);
  k_stencil_texture = kernel_memory_from_gltexture2D(stencil_texture);
  
  k_color_volume = kernel_memory_allocate(width * height * depth * 4 * sizeof(char));
  k_normals_volume = kernel_memory_allocate(width * height * depth * 4 * sizeof(char));
  
  kernel_memory_write(k_color_volume, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  kernel_memory_write(k_normals_volume, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  
  free(blank_byte_volume);
  free(blank_stencil_texture);
  free(blank_byte_texture);  
  
  kernel_program* rendering = asset_get("./kernels/volume_rendering.cl");
  kernel_program* blitting = asset_get("./kernels/volume_blitting.cl");
  
  k_clear_texture = kernel_program_get_kernel(rendering, "clear_texture");
  kernel_set_argument(k_clear_texture, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_clear_texture, 1, sizeof(cl_int), &height);
  
  k_clear_volume = kernel_program_get_kernel(rendering, "clear_volume");
  kernel_set_argument(k_clear_volume, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_clear_volume, 1, sizeof(cl_int), &height);
  kernel_set_argument(k_clear_volume, 2, sizeof(cl_int), &depth);
  
  k_flatten = kernel_program_get_kernel(rendering, "volume_flatten");
  kernel_set_argument(k_flatten, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_flatten, 1, sizeof(cl_int), &height);
  kernel_set_argument(k_flatten, 2, sizeof(cl_int), &depth);
  kernel_set_argument(k_flatten, 3, sizeof(cl_float), &threshold);
  kernel_set_argument(k_flatten, 4, sizeof(kernel_memory), &k_proj_texture);
  kernel_set_argument(k_flatten, 5, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_flatten, 6, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_flatten, 7, sizeof(kernel_memory), &k_normals_volume);
  
  k_blit_point = kernel_program_get_kernel(blitting, "blit_point");
  kernel_set_argument(k_blit_point, 4, sizeof(cl_int), &width);
  kernel_set_argument(k_blit_point, 5, sizeof(cl_int), &height);
  kernel_set_argument(k_blit_point, 6, sizeof(cl_int), &depth);
  kernel_set_argument(k_blit_point, 7, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_blit_point, 8, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_blit_point, 9, sizeof(kernel_memory), &k_normals_volume);
 
  k_blit_metaball = kernel_program_get_kernel(blitting, "blit_metaball");
  kernel_set_argument(k_blit_metaball, 7, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_blit_metaball, 8, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_blit_metaball, 9, sizeof(kernel_memory), &k_normals_volume);
}

void volume_renderer_finish() {
  
  kernel_memory_delete(k_proj_texture);
  kernel_memory_delete(k_stencil_texture);

  kernel_memory_delete(k_color_volume);
  kernel_memory_delete(k_normals_volume);
  
  glEnable(GL_TEXTURE_2D);
  glDeleteTextures(1, &proj_texture);
  glDeleteTextures(1, &stencil_texture);
  
}

void volume_renderer_begin() {
  
  vector4 zero = v4_zero();
  vector4 norm = v4(0.5, 0.5, 0.5, 0);
  
  kernel_set_argument(k_clear_volume, 3, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_clear_volume, 4, sizeof(vector4), &zero);
  kernel_run(k_clear_volume, width * height * depth);
  
  kernel_set_argument(k_clear_volume, 3, sizeof(kernel_memory), &k_normals_volume);
  kernel_set_argument(k_clear_volume, 4, sizeof(vector4), &norm);
  kernel_run(k_clear_volume, width * height * depth);
  
  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
    
    kernel_set_argument(k_clear_texture, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_set_argument(k_clear_texture, 3, sizeof(vector4), &zero);
    kernel_run(k_clear_texture, width * height);
    
    kernel_set_argument(k_clear_texture, 2, sizeof(kernel_memory), &k_proj_texture);
    kernel_set_argument(k_clear_texture, 3, sizeof(vector4), &zero);
    kernel_run(k_clear_texture, width * height);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  
}

void volume_renderer_end() {

  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
  
    vector4 light_position = v3_to_homogeneous(v3_sub(sun->position, v3_zero()));
    vector4 camera_position = v3_to_homogeneous(cam->position);
    kernel_set_argument(k_flatten, 8, sizeof(vector4), &light_position);
    kernel_set_argument(k_flatten, 9, sizeof(vector4), &camera_position);
    kernel_run(k_flatten, width * height);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, proj_texture);
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
  
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  
}

void volume_renderer_render_point(vector3 point, vector3 color) {
  
  kernel_memory_gl_aquire(k_stencil_texture);
  
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, viewport_ratio());  
  
    vector4 k_point = v4(point.x, point.y, point.z, 1);
    vector4 k_color = v4(color.x, color.y, color.z, 1);
    
    kernel_set_argument(k_blit_point, 0, sizeof(vector4), &k_point);
    kernel_set_argument(k_blit_point, 1, sizeof(vector4), &k_color);
    kernel_set_argument(k_blit_point, 2, sizeof(matrix_4x4), &viewm);
    kernel_set_argument(k_blit_point, 3, sizeof(matrix_4x4), &projm);
    kernel_run(k_blit_point, 1);
  
  kernel_memory_gl_release(k_stencil_texture);
  
}

static int multiple_two(int x) {
  int mul = 0;
  while(pow(2, mul) < x) {
    mul++;
  }
  return pow(2, mul);
}

void volume_renderer_render_metaball(vector3 position, vector3 color) {
  
  matrix_4x4 view_m = camera_view_matrix(cam);
  matrix_4x4 proj_m = camera_proj_matrix(cam, (float)height / (float)width); 
  matrix_4x4 inv_view_m = m44_inverse(view_m);
  matrix_4x4 inv_proj_m = m44_inverse(proj_m); 
  
  float b_size = 2;
  vector4 bounding_box[8] = { v4(b_size,b_size,b_size,1),  v4(-b_size,b_size,b_size,1),
                              v4(b_size,-b_size,b_size,1), v4(b_size,b_size,-b_size,1), 
                              v4(-b_size,-b_size,b_size,1), v4(b_size,-b_size,-b_size,1),
                              v4(-b_size,b_size,-b_size,1), v4(-b_size,-b_size,-b_size,1) };
                              
  for(int i = 0; i < 8; i++) {
    bounding_box[i] = v4_add(bounding_box[i], v4(position.x, position.y, position.z, 0));
    bounding_box[i] = m44_mul_v4(view_m, bounding_box[i]);
    bounding_box[i] = m44_mul_v4(proj_m, bounding_box[i]);
    vector3 t_position = v3(bounding_box[i].x, bounding_box[i].y, bounding_box[i].z);
    vector3 t_proj = v3_div(t_position, bounding_box[i].w);
    bounding_box[i] = v4(t_proj.x, t_proj.y, t_proj.z, 1);
  }
  
  float x_max, y_max, z_max = -1;
  float x_min, y_min, z_min = 1;
  
  for(int i = 0; i < 8; i++) {
    x_max = clamp(max(x_max, bounding_box[i].x), -1, 1);
    x_min = clamp(min(x_min, bounding_box[i].x), -1, 1);
    y_max = clamp(max(y_max, bounding_box[i].y), -1, 1);
    y_min = clamp(min(y_min, bounding_box[i].y), -1, 1);
    z_max = clamp(max(z_max, bounding_box[i].z), -1, 1);
    z_min = clamp(min(z_min, bounding_box[i].z), -1, 1);
  }
  
  int x_bot = ((float)x_min + 1) * 0.5 * width;
  int x_top = ((float)x_max + 1) * 0.5 * width;
  int y_bot = ((float)y_min + 1) * 0.5 * height;
  int y_top = ((float)y_max + 1) * 0.5 * height;
  int z_bot = ((float)z_min + 1) * 0.5 * depth;
  int z_top = ((float)z_max + 1) * 0.5 * depth;
  
  int x_size = x_top - x_bot;
  int y_size = y_top - y_bot;
  int z_size = z_top - z_bot;
  
  int padding = 1;
  
  int offset[3] = {x_bot-padding, y_bot-padding, z_bot-padding};
  int size[3] = {x_size+padding, y_size+padding, z_size+padding};
  int total_size[3] = {width, height, depth};
  
  //printf("Offset: %i %i %i\n", offset[0], offset[1], offset[2]);
  //printf("Size: %i %i %i\n", size[0], size[1], size[2]);
  
  kernel_memory_gl_aquire(k_stencil_texture);
  
    kernel_set_argument(k_blit_metaball, 0, sizeof(vector4), &position);
    kernel_set_argument(k_blit_metaball, 1, sizeof(vector4), &color);
    kernel_set_argument(k_blit_metaball, 2, sizeof(matrix_4x4), &inv_view_m);
    kernel_set_argument(k_blit_metaball, 3, sizeof(matrix_4x4), &inv_proj_m);
    kernel_set_argument(k_blit_metaball, 4, sizeof(cl_int3), &offset);
    kernel_set_argument(k_blit_metaball, 5, sizeof(cl_int3), &size);
    kernel_set_argument(k_blit_metaball, 6, sizeof(cl_int3), &total_size);
    int total_num = multiple_two(x_size + 2*padding) * multiple_two(y_size + 2*padding) * multiple_two(z_size + 2*padding);
    kernel_run(k_blit_metaball, total_num);
  
  kernel_memory_gl_release(k_stencil_texture);
}

