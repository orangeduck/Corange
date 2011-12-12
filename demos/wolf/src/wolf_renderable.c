#include "wolf_renderable.h"

#include "asset_manager.h"
#include "brush_dict.h"

#include <math.h>

wolf_renderable* wolf_renderable_new(char* name, model* m, texture* brush_texture, int num_brushes, vector2 brush_scale, float density) {
  
  wolf_renderable* w = malloc(sizeof(wolf_renderable));
  w->renderable = renderable_new(m);
  
  w->brush_texture = brush_texture;
  w->num_brushes = num_brushes;
  w->brush_scale = brush_scale;
  
  float surface_area = model_surface_area(m);
  w->num_particles = (int)(surface_area / density);
  
  printf("Generating %i particle strokes\n", w->num_particles);
  
  glGenBuffers(1, &w->brush_id_vbo);
  glGenBuffers(1, &w->position_vbo);
  glGenBuffers(1, &w->direction_vbo);
  
  float* brush_id_buffer = malloc( sizeof(float) * w->num_particles * 4 * 1);
  float* positions_buffer = malloc( sizeof(float) * w->num_particles * 4 * 3);
  float* direction_buffer = malloc( sizeof(float) * w->num_particles * 4 * 3);
  
  int brush_index = 0;
  int pos_index = 0;
  int dir_index = 0;
  
  float sum = 0.0;
  int placed = 0;
  
  image* ref_img = texture_get_image(asset_get("/resources/piano/piano.dds"));
  image* stencil = image_blank(ref_img->width, ref_img->height);
  
  brush_dict_init(brush_texture, 16, 16, 164);
  
  timer_start();
  
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
      count = sum / density;
      
      int kcount = floor(count);
      
      int k;
      for(k = 0; k < kcount; k++) {
        
        vertex rand_pos = triangle_random_position_interpolation(v1, v2, v3);
        
        vector3 position = rand_pos.position;
        vector2 uvs = rand_pos.uvs;
        vector3 normal = rand_pos.normal;
        vector3 tangent = rand_pos.tangent;
        
        int brush_id = brush_id_for_position(ref_img, stencil, uvs);
        
        /* Place particle */
        
          /* -------------------------- */
        
          brush_id_buffer[brush_index] = brush_id; brush_index++;
        
          positions_buffer[pos_index] = position.x; pos_index++;
          positions_buffer[pos_index] = position.y; pos_index++;
          positions_buffer[pos_index] = position.z; pos_index++;
                   
          direction_buffer[dir_index] = tangent.x; dir_index++;
          direction_buffer[dir_index] = tangent.y; dir_index++;
          direction_buffer[dir_index] = tangent.z; dir_index++;
        
          /* -------------------------- */

          brush_id_buffer[brush_index] = brush_id; brush_index++;
        
          positions_buffer[pos_index] = position.x; pos_index++;
          positions_buffer[pos_index] = position.y; pos_index++;
          positions_buffer[pos_index] = position.z; pos_index++;
                   
          direction_buffer[dir_index] = tangent.x; dir_index++;
          direction_buffer[dir_index] = tangent.y; dir_index++;
          direction_buffer[dir_index] = tangent.z; dir_index++;
      
          /* -------------------------- */
        
          brush_id_buffer[brush_index] = brush_id; brush_index++;
        
          positions_buffer[pos_index] = position.x; pos_index++;
          positions_buffer[pos_index] = position.y; pos_index++;
          positions_buffer[pos_index] = position.z; pos_index++;
                   
          direction_buffer[dir_index] = tangent.x; dir_index++;
          direction_buffer[dir_index] = tangent.y; dir_index++;
          direction_buffer[dir_index] = tangent.z; dir_index++;
        
          /* -------------------------- */
          
          brush_id_buffer[brush_index] = brush_id; brush_index++;
        
          positions_buffer[pos_index] = position.x; pos_index++;
          positions_buffer[pos_index] = position.y; pos_index++;
          positions_buffer[pos_index] = position.z; pos_index++;
                   
          direction_buffer[dir_index] = tangent.x; dir_index++;
          direction_buffer[dir_index] = tangent.y; dir_index++;
          direction_buffer[dir_index] = tangent.z; dir_index++;
          
          /* -------------------------- */
        
        /* End Place Particle */
        
      }
      sum = sum - (density * kcount);
      placed += kcount;
      
      j = j + 3;
    }
    
  }
  
  timer_stop();
  
  image_delete(ref_img);
  
  brush_dict_finish();
  
  glBindBuffer(GL_ARRAY_BUFFER, w->brush_id_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * w->num_particles * 4 * 1, brush_id_buffer, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, w->position_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * w->num_particles * 4 * 3, positions_buffer, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, w->direction_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * w->num_particles * 4 * 3, direction_buffer, GL_STATIC_DRAW);
  
  free(brush_id_buffer);
  free(positions_buffer);
  free(direction_buffer);
  
  printf("Done\n");fflush(stdout);
  
  return w;
}

void wolf_renderable_delete(wolf_renderable* w) {

  glDeleteBuffers(1, &w->brush_id_vbo);
  glDeleteBuffers(1, &w->position_vbo);
  glDeleteBuffers(1, &w->direction_vbo);

  renderable_delete(w->renderable);
  free(w);
}