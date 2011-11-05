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

int width = 256;
int depth = 256;
int height = 256;

float threshold = 0.0;

GLuint intensity_volume;
GLuint color_volume;
GLuint normals_volume;

GLuint proj_texture;
GLuint stencil_texture;

kernel_memory k_intensity_volume;
kernel_memory k_color_volume;
kernel_memory k_normals_volume;

kernel_memory k_proj_texture;
kernel_memory k_stencil_texture;

kernel k_flatten;

void volume_renderer_init() {
  
  float* blank_float_volume = calloc(width * height * depth, sizeof(float));
  char* blank_byte_volume = calloc(width * height * depth * 4, sizeof(char));
  
  glEnable(GL_TEXTURE_3D);
  glGenTextures(1, &intensity_volume);
  glBindTexture(GL_TEXTURE_3D, intensity_volume);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_FLOAT, blank_float_volume);
  SDL_CheckOpenGLError("glTexImage3D");
  
  glGenTextures(1, &color_volume);
  glBindTexture(GL_TEXTURE_3D, color_volume);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_volume);
  SDL_CheckOpenGLError("glTexImage3D");
  
  glGenTextures(1, &normals_volume);
  glBindTexture(GL_TEXTURE_3D, normals_volume);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank_byte_volume);
  SDL_CheckOpenGLError("glTexImage3D");
  
  free(blank_byte_volume);
  free(blank_float_volume);
  
  char* blank_byte_texture = calloc(width * height * 4, sizeof(char));
  char* blank_stencil_texture = calloc(width * height, sizeof(char));
  
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
  
  free(blank_stencil_texture);
  free(blank_byte_texture);
 
  k_intensity_volume = kernel_memory_from_gltexture3D(intensity_volume);
  k_color_volume = kernel_memory_from_gltexture3D(color_volume);
  k_normals_volume = kernel_memory_from_gltexture3D(normals_volume);
  
  k_proj_texture = kernel_memory_from_gltexture2D(proj_texture);
  k_stencil_texture = kernel_memory_from_gltexture2D(stencil_texture);
 
  kernel_program* program = asset_get("/kernels/volume_rendering.cl");
  
  k_flatten = kernel_program_get_kernel(program, "volume_flatten");
  kernel_set_argument(k_flatten, 0, sizeof(cl_int), &width);
  kernel_set_argument(k_flatten, 1, sizeof(cl_int), &height);
  kernel_set_argument(k_flatten, 2, sizeof(cl_int), &depth);
  kernel_set_argument(k_flatten, 3, sizeof(cl_float), &threshold);
  kernel_set_argument(k_flatten, 4, sizeof(kernel_memory), &k_proj_texture);
  kernel_set_argument(k_flatten, 5, sizeof(kernel_memory), &k_stencil_texture);
  kernel_set_argument(k_flatten, 6, sizeof(kernel_memory), &k_intensity_volume);
  kernel_set_argument(k_flatten, 7, sizeof(kernel_memory), &k_color_volume);
  kernel_set_argument(k_flatten, 8, sizeof(kernel_memory), &k_normals_volume);
 
}

void volume_renderer_finish() {

  kernel_memory_delete(k_intensity_volume);
  kernel_memory_delete(k_color_volume);
  kernel_memory_delete(k_normals_volume);
  
  kernel_memory_delete(k_proj_texture);
  kernel_memory_delete(k_stencil_texture);

  glEnable(GL_TEXTURE_3D);
  glDeleteTextures(1, &intensity_volume);
  glDeleteTextures(1, &color_volume);
  glDeleteTextures(1, &normals_volume);
  glDisable(GL_TEXTURE_3D);
  
  glEnable(GL_TEXTURE_2D);
  glDeleteTextures(1, &proj_texture);
  glDeleteTextures(1, &stencil_texture);
  
}

void volume_renderer_begin() {

}

void volume_renderer_end() {
  
  kernel_memory_gl_aquire(k_proj_texture);
  kernel_memory_gl_aquire(k_stencil_texture);
  kernel_memory_gl_aquire(k_intensity_volume);
  kernel_memory_gl_aquire(k_color_volume);
  kernel_memory_gl_aquire(k_normals_volume);
  
  kernel_run(k_flatten, width * height, width);
  
  kernel_memory_gl_release(k_proj_texture);
  kernel_memory_gl_release(k_stencil_texture);
  kernel_memory_gl_release(k_intensity_volume);
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

void volume_renderer_render_point(vector3 point, camera* cam) {
  
  matrix_4x4 view_matrix = camera_view_matrix(cam);
  matrix_4x4 proj_matrix = camera_proj_matrix(cam, viewport_ratio());
  
  
  
}

void volume_renderer_render_function() {
  
  
  
}

