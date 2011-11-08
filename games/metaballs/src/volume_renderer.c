#include "volume_renderer.h"

#include "corange.h"

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

int width = 256;
int depth = 32;
int height = 256;

float threshold = 0.0;

GLuint intensity_volume;
GLuint color_volume;
GLuint normals_volume;

GLuint proj_texture;
GLuint stencil_texture;

kernel_memory k_color_volume;
kernel_memory k_normals_volume;
kernel_memory k_proj_texture;
kernel_memory k_stencil_texture;

kernel_memory K_color_volume_buffer;
kernel_memory k_normals_volume_buffer;

kernel k_flatten;
kernel k_blit_point;
kernel k_clear_texture;

camera* cam = NULL;

char* blank_byte_volume;
char* blank_byte_texture;
char* blank_stencil_texture;

void volume_renderer_set_camera(camera* new_cam) {
  cam = new_cam;
}

void volume_renderer_init() {
    
  blank_byte_volume = calloc(width * height * depth * 4, sizeof(char));
  blank_byte_texture = calloc(width * height * 4, sizeof(char));
  blank_stencil_texture = calloc(width * height, sizeof(char));  
  
  glEnable(GL_TEXTURE_3D);
  
  glGenTextures(1, &color_volume);
  glBindTexture(GL_TEXTURE_3D, color_volume);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_volume);
  SDL_CheckOpenGLError("glTexImage3D");
  
  glGenTextures(1, &normals_volume);
  glBindTexture(GL_TEXTURE_3D, normals_volume);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_volume);
  SDL_CheckOpenGLError("glTexImage3D");
  
  glDisable(GL_TEXTURE_3D);
  
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &proj_texture);
  glBindTexture(GL_TEXTURE_2D, proj_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_texture);
  SDL_CheckOpenGLError("glTexImage2D");
  
  glGenTextures(1, &stencil_texture);
  glBindTexture(GL_TEXTURE_2D, stencil_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, blank_stencil_texture);
  SDL_CheckOpenGLError("glTexImage2D");
 
  k_color_volume = kernel_memory_from_gltexture3D(color_volume);
  k_normals_volume = kernel_memory_from_gltexture3D(normals_volume);
  
  k_proj_texture = kernel_memory_from_gltexture2D(proj_texture);
  k_stencil_texture = kernel_memory_from_gltexture2D(stencil_texture);
  
  K_color_volume_buffer = kernel_memory_allocate(width * height * depth * 4 * sizeof(char));
  k_normals_volume_buffer = kernel_memory_allocate(width * height * depth * 4 * sizeof(char));
  
  kernel_memory_write(K_color_volume_buffer, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  kernel_memory_write(k_normals_volume_buffer, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  
  k_clear_texture = kernel_program_get_kernel(asset_get("/kernels/volume_rendering.cl"), "clear_texture");
  kernel_set_argument(k_clear_texture, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_clear_texture, 1, sizeof(cl_int), &height);
  
  k_flatten = kernel_program_get_kernel(asset_get("/kernels/volume_rendering.cl"), "volume_flatten");
  kernel_set_argument(k_flatten, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_flatten, 1, sizeof(cl_int), &height);
  kernel_set_argument(k_flatten, 2, sizeof(cl_int), &depth);
  kernel_set_argument(k_flatten, 3, sizeof(cl_float), &threshold);
  kernel_set_argument(k_flatten, 4, sizeof(kernel_memory), &k_proj_texture);
  kernel_set_argument(k_flatten, 5, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_flatten, 6, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_flatten, 7, sizeof(kernel_memory), &k_normals_volume);
  
  k_blit_point = kernel_program_get_kernel(asset_get("/kernels/volume_blitting.cl"), "blit_point");
  kernel_set_argument(k_blit_point, 4, sizeof(cl_int), &width);
  kernel_set_argument(k_blit_point, 5, sizeof(cl_int), &height);
  kernel_set_argument(k_blit_point, 6, sizeof(cl_int), &depth);
  kernel_set_argument(k_blit_point, 7, sizeof(kernel_memory), &k_proj_texture);
  kernel_set_argument(k_blit_point, 8, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_blit_point, 9, sizeof(kernel_memory), &K_color_volume_buffer);
  kernel_set_argument(k_blit_point, 10, sizeof(kernel_memory), &k_normals_volume_buffer);
 
}

void volume_renderer_finish() {

  kernel_memory_delete(k_color_volume);
  kernel_memory_delete(k_normals_volume);
  
  kernel_memory_delete(k_proj_texture);
  kernel_memory_delete(k_stencil_texture);

  kernel_memory_delete(K_color_volume_buffer);
  kernel_memory_delete(k_normals_volume_buffer);
  
  glEnable(GL_TEXTURE_3D);
  glDeleteTextures(1, &intensity_volume);
  glDeleteTextures(1, &color_volume);
  glDeleteTextures(1, &normals_volume);
  glDisable(GL_TEXTURE_3D);
  
  glEnable(GL_TEXTURE_2D);
  glDeleteTextures(1, &proj_texture);
  glDeleteTextures(1, &stencil_texture);
  
  free(blank_byte_volume);
  free(blank_stencil_texture);
  free(blank_byte_texture);
  
}

void volume_renderer_begin() {

  kernel_memory_write(K_color_volume_buffer, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  kernel_memory_write(k_normals_volume_buffer, width * height * depth * 4 * sizeof(char), blank_byte_volume);
  
  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
  
    kernel_set_argument(k_clear_texture, 2, sizeof(kernel_memory), &k_stencil_texture);
    kernel_run(k_clear_texture, width * height, width);
    
    kernel_set_argument(k_clear_texture, 2, sizeof(kernel_memory), &k_proj_texture);
    kernel_run(k_clear_texture, width * height, width);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  
}

void volume_renderer_end() {
  
  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
  kernel_memory_gl_aquire(k_color_volume);
  kernel_memory_gl_aquire(k_normals_volume);
  
    kernel_memory_copy_to_texture(K_color_volume_buffer, k_color_volume, width, height, depth);
    kernel_memory_copy_to_texture(k_normals_volume_buffer, k_normals_volume, width, height, depth);
  
    kernel_run(k_flatten, width * height, width);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  kernel_memory_gl_release(k_color_volume);
  kernel_memory_gl_release(k_normals_volume);
  
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
  
  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
  
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, viewport_ratio());  
  
    vector4 k_point = v4(point.x, point.y, point.z, 1);
    vector4 k_color = v4(color.x, color.y, color.z, 1);
    
    kernel_set_argument(k_blit_point, 0, sizeof(vector4), &k_point);
    kernel_set_argument(k_blit_point, 1, sizeof(vector4), &k_color);
    kernel_set_argument(k_blit_point, 2, sizeof(matrix_4x4), &viewm);
    kernel_set_argument(k_blit_point, 3, sizeof(matrix_4x4), &projm);
    kernel_run(k_blit_point, 1, 1);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  
}

void volume_renderer_render_metaball(vector3 position, vector3 color) {
  
  
  
}

