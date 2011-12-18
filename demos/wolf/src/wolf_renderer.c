#include "wolf_renderer.h"

static camera* CAMERA = NULL;
static light* LIGHT = NULL;
static texture* SHADOW_TEX = NULL;

static float PROJ_MATRIX[16];
static float VIEW_MATRIX[16];
static float WORLD_MATRIX[16];

static float LIGHT_VIEW_MATRIX[16];
static float LIGHT_PROJ_MATRIX[16];

static shader_program* DEFERRED_PROGRAM;
static shader_program* COMPOSITION_PROGRAM_HDR;
static shader_program* COMPOSITION_PROGRAM_LDR;
static shader_program* PAINTING_PROGRAM;

static int NORMAL_ATTRIBUTE;
static int TANGENT_ATTRIBUTE;
static int BINORMAL_ATTRIBUTE;
static int COLOR_ATTRIBUTE;

static int BRUSH_ID_ATTRIBUTE;
static int FACE_POSITION_ATTRIBUTE;
static int FACE_DIRECTION_ATTRIBUTE;

static int DOWNSCALE = 4;

/* Buffers for deferred pass */

/* In this pass we do a normal deferred rendering pass, rendering to the gbuffer. */


static GLuint gbuffer_fbo;

static GLuint depth_buffer;
static GLuint diffuse_buffer;
static GLuint positions_buffer;
static GLuint normals_buffer;

static GLuint diffuse_texture;
static GLuint positions_texture;
static GLuint normals_texture;
static GLuint depth_texture;

/* Buffers for composition pass */

/* In this pass we compose the data from the gbuffer, shadow texture etc, into a complete color output. Is written to a HDR buffer and a bloom buffer. */

static GLuint composition_hdr_fbo;
static GLuint composed_buffer_hdr;
static GLuint composed_texture_hdr;

/* We then tone map the HDR into a LDR texture. This includes color correction stages and adding bloom */

static GLuint composition_ldr_fbo;
static GLuint composed_buffer_ldr;
static GLuint composed_texture_ldr;

/* Buffers for painting pass */

/* In this pass we use the composed color data to paint onto the scene. This includes brush strokes for normal models as well as other post painting effects such as volumetric fog and individual strokes for flora and fauna */

static GLuint painting_fbo;
static GLuint painting_buffer;
static GLuint painting_texture;

/* Particle stuff */

#define MAX_PARTICLES 1000000

GLuint particle_positions_vbo;
GLuint particle_uvs_vbo;

GLuint* particle_index_vbos;
int num_particle_index_vbos;

/*

Then we store for each particle on each model
  
  Brush index (short) x 4
  Position (float,float,float) x 4
  Direction (float,float,float) x 4
  
*/

void wolf_renderer_init() {
  
  DEFERRED_PROGRAM = asset_get("/resources/shaders/deferred.prog");
  COMPOSITION_PROGRAM_HDR = asset_get("/resources/shaders/deferred_screen.prog");
  COMPOSITION_PROGRAM_LDR = asset_get("/resources/shaders/deferred_tonemap.prog");
  PAINTING_PROGRAM = asset_get("/resources/shaders/painting.prog");
  
  NORMAL_ATTRIBUTE = glGetAttribLocation(*DEFERRED_PROGRAM, "vNormal");
  TANGENT_ATTRIBUTE = glGetAttribLocation(*DEFERRED_PROGRAM, "vTangent");
  BINORMAL_ATTRIBUTE = glGetAttribLocation(*DEFERRED_PROGRAM, "vBiNormal");
  COLOR_ATTRIBUTE = glGetAttribLocation(*DEFERRED_PROGRAM, "vColor");
  
  BRUSH_ID_ATTRIBUTE = glGetAttribLocation(*PAINTING_PROGRAM, "brush_id");
  FACE_POSITION_ATTRIBUTE = glGetAttribLocation(*PAINTING_PROGRAM, "face_position");
  FACE_DIRECTION_ATTRIBUTE = glGetAttribLocation(*PAINTING_PROGRAM, "face_direction");
  
  const int reduced_width = viewport_width() / DOWNSCALE;
  const int reduced_height = viewport_height() / DOWNSCALE;
  
  /* Gbuffer pass */
  
  glGenFramebuffers(1, &gbuffer_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo);
  
  glGenRenderbuffers(1, &depth_buffer);
  glGenRenderbuffers(1, &diffuse_buffer);
  glGenRenderbuffers(1, &positions_buffer);
  glGenRenderbuffers(1, &normals_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, diffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, reduced_width, reduced_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glBindRenderbuffer(GL_RENDERBUFFER, positions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, reduced_width, viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, positions_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, normals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, reduced_width, viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, normals_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, reduced_width, viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reduced_width, reduced_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, reduced_width, reduced_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positions_texture, 0);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, reduced_width, reduced_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normals_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, reduced_width, reduced_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
  /* HDR Composition */
  
  glGenFramebuffers(1, &composition_hdr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, composition_hdr_fbo);
  
  glBindRenderbuffer(GL_RENDERBUFFER, composed_buffer_hdr);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, reduced_width, reduced_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, composed_buffer_hdr); 

  glGenTextures(1, &composed_texture_hdr);
  glBindTexture(GL_TEXTURE_2D, composed_texture_hdr);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, reduced_width, reduced_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, composed_texture_hdr, 0);
  
  /* LDR Composition */
  
  glGenFramebuffers(1, &composition_ldr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, composition_ldr_fbo);
  
  glBindRenderbuffer(GL_RENDERBUFFER, composed_buffer_ldr);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, reduced_width, reduced_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, composed_buffer_ldr); 
  
  glGenTextures(1, &composed_texture_ldr);
  glBindTexture(GL_TEXTURE_2D, composed_texture_ldr);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reduced_width, reduced_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, composed_texture_ldr, 0);
  
  /* Painting */
  
  glGenFramebuffers(1, &painting_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, painting_fbo);
  
  glBindRenderbuffer(GL_RENDERBUFFER, painting_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height() );
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, painting_buffer); 
  
  glGenTextures(1, &painting_texture);
  glBindTexture(GL_TEXTURE_2D, painting_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, painting_texture, 0);
  
  glGenBuffers(1, &particle_positions_vbo);
  glGenBuffers(1, &particle_uvs_vbo);
  
  num_particle_index_vbos = 10;
  particle_index_vbos = malloc(sizeof(GLuint) * num_particle_index_vbos);
  glGenBuffers(num_particle_index_vbos, particle_index_vbos);
  
  float* particle_positions = malloc(sizeof(float) * 4 * 3 * MAX_PARTICLES);
  float* particle_uvs = malloc(sizeof(float) * 4 * 2 * MAX_PARTICLES);
  
  unsigned long i = 0;
  
  while( i < MAX_PARTICLES * 3 * 4) {
    particle_positions[i  ] = -1;
    particle_positions[i+1] = -1;
    particle_positions[i+2] =  0;
    
    particle_positions[i+3] = -1;
    particle_positions[i+4] =  1;
    particle_positions[i+5] =  0;
    
    particle_positions[i+6] = 1;
    particle_positions[i+7] = 1;
    particle_positions[i+8] = 0;
    
    particle_positions[i+9 ] =  1;
    particle_positions[i+10] = -1;
    particle_positions[i+11] =  0;
    
    i += 3 * 4;
  }
  
  i = 0;
  while(i < MAX_PARTICLES * 2 * 4) {
    particle_uvs[i  ] = 0;
    particle_uvs[i+1] = 0;
    
    particle_uvs[i+2] = 0;
    particle_uvs[i+3] = 1;
    
    particle_uvs[i+4] = 1;
    particle_uvs[i+5] = 1;
    
    particle_uvs[i+6] = 1;
    particle_uvs[i+7] = 0;
    
    i += 8;
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, particle_positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_PARTICLES * 4 * 3, particle_positions, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, particle_uvs_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_PARTICLES * 4 * 2, particle_uvs, GL_STATIC_DRAW);
  
  free(particle_positions);
  free(particle_uvs);
  
  for(i = 0; i < num_particle_index_vbos; i++) {
  
    int skip = (i+1) * (i+1);
    
    int num_indicies = (MAX_PARTICLES / skip);
    
    int* index_data = malloc( sizeof(int) * num_indicies );
    int j;
    for( j = 0; j < num_indicies; j++) {
      index_data[j] = j * skip;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_index_vbos[i]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indicies, index_data, GL_STATIC_DRAW);
    
    free(index_data);
  
  }
  
}

void wolf_renderer_finish() {
  
  /* Gbuffer */
  
  glDeleteFramebuffers(1, &gbuffer_fbo);
  
  glDeleteRenderbuffers(1, &diffuse_buffer);
  glDeleteRenderbuffers(1, &positions_buffer);
  glDeleteRenderbuffers(1, &normals_buffer);
  glDeleteRenderbuffers(1, &depth_buffer);
  
  glDeleteTextures(1,&diffuse_texture);
  glDeleteTextures(1,&positions_texture);
  glDeleteTextures(1,&normals_texture);
  glDeleteTextures(1,&depth_texture);

  /* HDR composition */
  
  glDeleteFramebuffers(1, &composition_hdr_fbo);
  glDeleteRenderbuffers(1, &composed_buffer_hdr);
  glDeleteTextures(1,&composed_texture_hdr);

  /* LDR Image */
  
  glDeleteFramebuffers(1, &composition_ldr_fbo);
  glDeleteRenderbuffers(1, &composed_buffer_ldr);
  glDeleteTextures(1,&composed_texture_ldr);
  
  /* Painting */
  
  glDeleteFramebuffers(1, &painting_fbo);
  glDeleteRenderbuffers(1, &painting_buffer);
  glDeleteTextures(1,&painting_texture);
  
}

void wolf_renderer_set_camera(camera* c) {
  CAMERA = c;
};

void wolf_renderer_set_light(light* l) {
  LIGHT = l;
}

void wolf_renderer_set_shadow_texture(texture* t) {
  SHADOW_TEX = t;
}

static void wolf_renderer_use_material(material* mat) {
  
  /* Set material parameters */
  
  int tex_counter = 0;
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*DEFERRED_PROGRAM, key);
    
    GLint world_matrix_u = glGetUniformLocation(*DEFERRED_PROGRAM, "world_matrix");
    glUniformMatrix4fv(world_matrix_u, 1, 0, WORLD_MATRIX);
    
    if (*type == mat_type_texture) {
    
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, *((texture*)property));
      tex_counter++;
    
    } else if (*type == mat_type_int) {
    
      glUniform1i(loc, *((float*)property));
    
    } else if (*type == mat_type_float) {
    
      glUniform1f(loc, *((float*)property));
      
    } else if (*type == mat_type_vector2) {
    
      vector2 v = *((vector2*)property);
      glUniform2f(loc, v.x, v.y);
    
    } else if (*type == mat_type_vector3) {
    
      vector3 v = *((vector3*)property);
      glUniform3f(loc, v.x, v.y, v.z);
  
    } else if (*type == mat_type_vector4) {
    
      vector4 v = *((vector4*)property);
      glUniform4f(loc, v.w, v.x, v.y, v.z);
    
    } else {
      /* Do nothing */
    }
  }  
}

static void wolf_renderer_setup_camera() {

  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
  
  m44_to_array(viewm, VIEW_MATRIX);
  m44_to_array(projm, PROJ_MATRIX);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(VIEW_MATRIX);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(PROJ_MATRIX);

  matrix_4x4 lviewm = light_view_matrix(LIGHT);
  matrix_4x4 lprojm = light_proj_matrix(LIGHT);
  
  m44_to_array(lviewm, LIGHT_VIEW_MATRIX);
  m44_to_array(lprojm, LIGHT_PROJ_MATRIX);
  
};

void wolf_renderer_begin() {

  wolf_renderer_setup_camera();

  glViewport(0, 0, viewport_width() / DOWNSCALE, viewport_height() / DOWNSCALE);
  
  glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo);
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, buffers);
  
  glDisable(GL_LIGHTING);
  
};

void wolf_renderer_end() {
  
  /* Disable various testings. Setup camera for quad drawing */
  
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  /* Draw to HDR texture */
  
  glBindFramebuffer(GL_FRAMEBUFFER, composition_hdr_fbo);
  glUseProgram(*COMPOSITION_PROGRAM_HDR);
  
  GLenum hdr_buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, hdr_buffers);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "positions_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "normals_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "depth_texture"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glBindTexture(GL_TEXTURE_2D, *SHADOW_TEX);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "shadows_texture"), 4);
  
  GLint cam_position = glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "camera_position");
  glUniform3f(cam_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  GLint light_position = glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "light_position");
  glUniform3f(light_position, LIGHT->position.x, LIGHT->position.y, LIGHT->position.z);
  
  GLint lproj_matrix_u = glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, LIGHT_PROJ_MATRIX);
  
  GLint lview_matrix_u = glGetUniformLocation(*COMPOSITION_PROGRAM_HDR, "light_view");
  glUniformMatrix4fv(lview_matrix_u, 1, 0, LIGHT_VIEW_MATRIX);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glDisable(GL_TEXTURE_2D);
  
  /* Draw to LDR texture */
  
  glBindFramebuffer(GL_FRAMEBUFFER, composition_ldr_fbo);
  glUseProgram(*COMPOSITION_PROGRAM_LDR);
  
  GLenum ldr_buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, ldr_buffers);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, composed_texture_hdr);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*COMPOSITION_PROGRAM_LDR, "hdr_texture"), 0);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);

}

void wolf_renderer_draw(wolf_renderable* wr) {

  renderable* r = wr->renderable;

  //matrix_4x4 r_world_matrix = m44_world( r->position, r->scale, r->rotation );
  matrix_4x4 r_world_matrix = m44_id();
  m44_to_array(r_world_matrix, WORLD_MATRIX);
  
  int i;
  for(i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
          
    glUseProgram(*DEFERRED_PROGRAM);
    
    wolf_renderer_use_material(s->base);
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
        
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
    
    glVertexAttribPointer(TANGENT_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(TANGENT_ATTRIBUTE);
    
    glVertexAttribPointer(BINORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
    glEnableVertexAttribArray(BINORMAL_ATTRIBUTE);
    
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    
    glDisableVertexAttribArray(NORMAL_ATTRIBUTE);
    glDisableVertexAttribArray(TANGENT_ATTRIBUTE);
    glDisableVertexAttribArray(BINORMAL_ATTRIBUTE);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(0);

  }

}

void wolf_renderer_begin_painting() {

  glBindFramebuffer(GL_FRAMEBUFFER, painting_fbo);
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  GLenum hdr_buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, hdr_buffers);
  
  /* Draw composed ldr texture to screen */
  
  glViewport(0, 0, viewport_width(), viewport_height());
  glUseProgram(0);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, composed_texture_ldr);
  glEnable(GL_TEXTURE_2D);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  /* Finish */
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(*PAINTING_PROGRAM);
  
  glBindTexture(GL_TEXTURE_2D, composed_texture_ldr);
  glGenerateMipmap(GL_TEXTURE_2D);
  
}

void wolf_renderer_paint(wolf_renderable* wr) {
  
  GLint world_matrix_u = glGetUniformLocation(*PAINTING_PROGRAM, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, WORLD_MATRIX);
  
  GLint proj_matrix_u = glGetUniformLocation(*PAINTING_PROGRAM, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, PROJ_MATRIX);
  
  GLint view_matrix_u = glGetUniformLocation(*PAINTING_PROGRAM, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, VIEW_MATRIX);
  
  GLint eye_position = glGetUniformLocation(*PAINTING_PROGRAM, "eye_position");
  glUniform3f(eye_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  

  glUniform1i(glGetUniformLocation(*PAINTING_PROGRAM, "background_color"), 0);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, composed_texture_ldr);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROGRAM, "background_depth"), 1);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROGRAM, "background_normal"), 2);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROGRAM, "brush"), 3);
  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_2D, *wr->brush_texture);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROGRAM, "num_brushes") , wr->num_brushes);
  
  /* Bind VBOs */
  
  glBindBuffer(GL_ARRAY_BUFFER, particle_positions_vbo);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, particle_uvs_vbo);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  
  glBindBuffer(GL_ARRAY_BUFFER, wr->brush_id_vbo);
  glVertexAttribPointer(BRUSH_ID_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(BRUSH_ID_ATTRIBUTE);
  
  glBindBuffer(GL_ARRAY_BUFFER, wr->position_vbo);
  glVertexAttribPointer(FACE_POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(FACE_POSITION_ATTRIBUTE);
  
  glBindBuffer(GL_ARRAY_BUFFER, wr->direction_vbo);
  glVertexAttribPointer(FACE_DIRECTION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(FACE_DIRECTION_ATTRIBUTE);

  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_index_vbos[0]);
  glDrawElements(GL_QUADS, wr->num_particles * 4, GL_UNSIGNED_INT, 0);

}

void wolf_renderer_end_painting() {

  /* Draw to screen */
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, viewport_width(), viewport_height());
  glUseProgram(0);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, painting_texture);
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
  
  /* Set back to normal */
  
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  
  glEnable(GL_CULL_FACE);
  
}