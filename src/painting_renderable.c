#include "painting_renderable.h"

#include <math.h>

#include "asset_manager.h"

/* vec3 ParticlePosition | vec2 ParticleUVs | vec3 FacePosition | vec3 FaceNormal */

painting_renderable* painting_renderable_new(char* name, float density, vector2 brush_size, texture* brush) {
  
  painting_renderable* pr = malloc(sizeof(painting_renderable));
  pr->renderable = renderable_new(name);
  pr->density = density;
  pr->brush_size = brush_size;
  pr->brush = brush;
  
  return pr;
  
}

void painting_renderable_delete(painting_renderable* pr) {

  glDeleteBuffers(1, &pr->position_vbo);
  glDeleteBuffers(1, &pr->uvs_vbo);
  glDeleteBuffers(1, &pr->face_position_vbo);
  glDeleteBuffers(1, &pr->face_normal_vbo);
  glDeleteBuffers(1, &pr->face_tangent_vbo);
  
  glDeleteBuffers(pr->num_index_vbos, pr->index_vbos);
  
  renderable_delete(pr->renderable);
  free(pr);

}

void painting_renderable_add_model(painting_renderable* pr, model* m) {

  renderable_add_model(pr->renderable, m);
  
  float surface_area = model_surface_area(m);
  
  pr->num_particles = (int)(surface_area / pr->density);
  
  printf("Generating %i particle strokes\n", pr->num_particles);
  
  glGenBuffers(1, &pr->position_vbo);
  glGenBuffers(1, &pr->uvs_vbo);
  glGenBuffers(1, &pr->face_position_vbo);
  glGenBuffers(1, &pr->face_normal_vbo);
  glGenBuffers(1, &pr->face_tangent_vbo);
  
  float* positions_buffer = malloc( sizeof(float) * pr->num_particles * 4 * 3 );
  float* uvs_buffer = malloc( sizeof(float) * pr->num_particles * 4 * 2);
  float* face_positions_buffer = malloc( sizeof(float) * pr->num_particles * 4 * 3);
  float* face_normals_buffer = malloc( sizeof(float) * pr->num_particles * 4 * 3);
  float* face_tangents_buffer = malloc( sizeof(float) * pr->num_particles * 4 * 3);
  
  pr->num_index_vbos = 15;
  pr->index_vbos = malloc( sizeof(GLuint) * pr->num_index_vbos );
  glGenBuffers(pr->num_index_vbos, pr->index_vbos);
  
  int pos_index = 0;
  int uvs_index = 0;
  int fpos_index = 0;
  int fnorm_index = 0;
  int ftan_index = 0;
  
  float sum = 0.0;
  int placed = 0;
  
  srand(time(NULL));
  
  int i;
  for( i = 0; i < m->num_meshes; i++) {
    mesh* me = m->meshes[i];
    
    int j = 0;
    while( j < me->num_triangles_3) {
    
      int t_i1 = me->triangles[j];
      int t_i2 = me->triangles[j+1];
      int t_i3 = me->triangles[j+2];

      vertex v1 = me->verticies[t_i1];
      vertex v2 = me->verticies[t_i2];
      vertex v3 = me->verticies[t_i3];
      
      sum += triangle_area(v1, v2, v3);
      
      double count;
      count = sum / pr->density;
      
      int kcount = floor(count);
      
      int k;
      for(k = 0; k < kcount; k++) {
        
        vector3 position = triangle_random_position(v1, v2, v3);
        vector3 normal = triangle_normal(v1, v2, v3);
        vector3 tangent;
        
        float u_diff = triangle_difference_u(v1, v2, v3);
        float v_diff = triangle_difference_v(v1, v2, v3);
        
        if(u_diff > v_diff) {
          tangent = triangle_tangent(v1, v2, v3);
        } else {
          tangent = triangle_binormal(v1, v2, v3);
        }
        
        /* Place particle */
        
          /* -------------------------- */
        
          positions_buffer[pos_index] = -1.0f; pos_index++;
          positions_buffer[pos_index] = -1.0f; pos_index++;
          positions_buffer[pos_index] = 0.0f; pos_index++;
          
          uvs_buffer[uvs_index] = 0.0f; uvs_index++;
          uvs_buffer[uvs_index] = 0.0f; uvs_index++;
          
          face_positions_buffer[fpos_index] = position.x; fpos_index++;
          face_positions_buffer[fpos_index] = position.y; fpos_index++;
          face_positions_buffer[fpos_index] = position.z; fpos_index++;
          
          face_normals_buffer[fnorm_index] = normal.x; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.y; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.z; fnorm_index++;
          
          face_tangents_buffer[ftan_index] = tangent.x; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.y; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.z; ftan_index++;
        
          /* -------------------------- */

          positions_buffer[pos_index] = -1.0f; pos_index++;
          positions_buffer[pos_index] = 1.0f; pos_index++;
          positions_buffer[pos_index] = 0.0f; pos_index++;
          
          uvs_buffer[uvs_index] = 0.0f; uvs_index++;
          uvs_buffer[uvs_index] = 1.0f; uvs_index++;
          
          face_positions_buffer[fpos_index] = position.x; fpos_index++;
          face_positions_buffer[fpos_index] = position.y; fpos_index++;
          face_positions_buffer[fpos_index] = position.z; fpos_index++;
          
          face_normals_buffer[fnorm_index] = normal.x; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.y; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.z; fnorm_index++;
        
          face_tangents_buffer[ftan_index] = tangent.x; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.y; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.z; ftan_index++;      
      
          /* -------------------------- */
        
          positions_buffer[pos_index] = 1.0f; pos_index++;
          positions_buffer[pos_index] = 1.0f; pos_index++;
          positions_buffer[pos_index] = 0.0f; pos_index++;
          
          uvs_buffer[uvs_index] = 1.0f; uvs_index++;
          uvs_buffer[uvs_index] = 1.0f; uvs_index++;
          
          face_positions_buffer[fpos_index] = position.x; fpos_index++;
          face_positions_buffer[fpos_index] = position.y; fpos_index++;
          face_positions_buffer[fpos_index] = position.z; fpos_index++;
          
          face_normals_buffer[fnorm_index] = normal.x; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.y; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.z; fnorm_index++;
        
          face_tangents_buffer[ftan_index] = tangent.x; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.y; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.z; ftan_index++;
        
          /* -------------------------- */
          
          positions_buffer[pos_index] = 1.0f; pos_index++;
          positions_buffer[pos_index] = -1.0f; pos_index++;
          positions_buffer[pos_index] = 0.0f; pos_index++;
          
          uvs_buffer[uvs_index] = 1.0f; uvs_index++;
          uvs_buffer[uvs_index] = 0.0f; uvs_index++;
          
          face_positions_buffer[fpos_index] = position.x; fpos_index++;
          face_positions_buffer[fpos_index] = position.y; fpos_index++;
          face_positions_buffer[fpos_index] = position.z; fpos_index++;
          
          face_normals_buffer[fnorm_index] = normal.x; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.y; fnorm_index++;
          face_normals_buffer[fnorm_index] = normal.z; fnorm_index++;
          
          face_tangents_buffer[ftan_index] = tangent.x; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.y; ftan_index++;
          face_tangents_buffer[ftan_index] = tangent.z; ftan_index++;
          
          /* -------------------------- */
        
        /* End Place Particle */
        
      }
      sum = sum - (pr->density * kcount);
      placed++;
      
      j = j + 3;
    }
    
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->position_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pr->num_particles * 4 * 3, positions_buffer, GL_STATIC_DRAW);
  free(positions_buffer);

  glBindBuffer(GL_ARRAY_BUFFER, pr->uvs_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pr->num_particles * 4 * 2, uvs_buffer, GL_STATIC_DRAW);
  free(uvs_buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_position_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pr->num_particles * 4 * 3, face_positions_buffer, GL_STATIC_DRAW);
  free(face_positions_buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pr->num_particles * 4 * 3, face_normals_buffer, GL_STATIC_DRAW);
  free(face_normals_buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_tangent_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pr->num_particles * 4 * 3, face_tangents_buffer, GL_STATIC_DRAW);
  free(face_tangents_buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  for(i = 0; i < pr->num_index_vbos; i++) {
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pr->index_vbos[i]);
    
    int skip = (i+1)*(i+1);
    int num_indicies = (pr->num_particles * 4) / (skip);
    
    //printf("Num Particles: %i\n", pr->num_particles);
    //printf("Num Indicies for skip %i: %i\n", i, num_indicies);
    
    int* index_buffer = malloc( sizeof(int) * num_indicies );
    
    int counter = 0;
    int j;
    for(j = 0; j < num_indicies-4; j+=4) {
      
      index_buffer[j] = counter; counter++;
      index_buffer[j+1] = counter; counter++;
      index_buffer[j+2] = counter; counter++;
      index_buffer[j+3] = counter; counter++;
      
      //printf("%i|", j+3);
      
      counter += 4 * skip;
    }
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_indicies, index_buffer, GL_STATIC_DRAW);
    free(index_buffer);
  }
  

}
