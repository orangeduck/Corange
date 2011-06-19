#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "asset_manager.h"

#include "renderable.h"

renderable* renderable_new(char* name) {
  
  renderable* r = malloc(sizeof(renderable));
  
  r->name = malloc(strlen(name) + 1);
  strcpy(r->name, name);
  
  r->num_surfaces = 0;
  r->surfaces = NULL;
  r->position = v3_zero();
  r->scale = v3_one();
  r->rotation = v4_quaternion_id();
  
  int active = 1;
  
  return r;

}

void renderable_delete(renderable* r) {

  free(r->name);
  int i;
  for(i = 0; i < r->num_surfaces; i++) {
    renderable_surface_delete( r->surfaces[i] );
  }
  
  free(r);

}

void renderable_add_model(renderable* r, model* m) {

  int i;
  for(i = 0; i < m->num_meshes; i++) {
    renderable_add_mesh(r, m->meshes[i]);
  }
  
}

void renderable_add_mesh(renderable* r, mesh* m) {
  
  renderable_surface* surface = renderable_surface_new(m);
  
  r->num_surfaces++;
  r->surfaces = realloc(r->surfaces, sizeof(renderable_surface*) *  r->num_surfaces);
  r->surfaces[r->num_surfaces-1] = surface;
}

void renderable_set_material(renderable* r, material* m) {
  int i;
  for(i = 0; i < r->num_surfaces; r++) {
    renderable_surface_set_material(r->surfaces[0], m);
  }
}

renderable_surface* renderable_surface_new(mesh* m) {

  renderable_surface* s = malloc(sizeof(renderable_surface));

  glGenBuffers(1, &s->vertex_vbo);
  glGenBuffers(1, &s->triangle_vbo);
  
  s->num_verticies = m->num_verts;
  s->num_triangles = m->num_triangles;
  
  /* Position Normal Tangent Binormal Uvs Color      */
  /* 3        3      3       3        2    4    = 18 */
  float* vb_data = malloc(sizeof(float) * m->num_verts * 18);
  
  int i;
  for(i = 0; i < m->num_verts; i++) {
  
    vector3 pos = m->verticies[i].position;
    vector3 norm = m->verticies[i].normal;
    vector3 tang = m->verticies[i].tangent;
    vector3 bino = m->verticies[i].binormal;
    vector2 uvs = m->verticies[i].uvs;
    vector4 col = m->verticies[i].color;
    
    vb_data[(i*18)+0] = pos.x;
    vb_data[(i*18)+1] = pos.y;
    vb_data[(i*18)+2] = pos.z;
    
    vb_data[(i*18)+3] = norm.x;
    vb_data[(i*18)+4] = norm.y;
    vb_data[(i*18)+5] = norm.z;
    
    vb_data[(i*18)+6] = tang.x;
    vb_data[(i*18)+7] = tang.y;
    vb_data[(i*18)+8] = tang.z;
    
    vb_data[(i*18)+9] = bino.x;
    vb_data[(i*18)+10] = bino.y;
    vb_data[(i*18)+11] = bino.z;
    
    vb_data[(i*18)+12] = uvs.x;
    vb_data[(i*18)+13] = uvs.y;
    
    vb_data[(i*18)+14] = col.w;
    vb_data[(i*18)+15] = col.x;
    vb_data[(i*18)+16] = col.y;
    vb_data[(i*18)+17] = col.z;
  
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s->num_verticies * 18, vb_data, GL_STATIC_DRAW);
  
  free(vb_data);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * s->num_triangles * 3, m->triangles, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  material* base = asset_get(m->material);
  
  //printf("mat: |%s|\n", m->material); fflush(stdout);
  
  if( base != NULL ) {
    s->base = base;
  } else {
    //s->base = asset_get("/resources/cello/cello.mat");
    s->base = asset_get("./engine/resources/basic.mat");
  }
    
  /* Copy certain base attributes from base material which can't be specified as non-existant */
  s->instance = material_new();
  s->instance->use_blending = s->base->use_blending;
  s->instance->src_blend_func = s->base->src_blend_func;
  s->instance->dst_blend_func = s->base->dst_blend_func;
  
  return s;
}

void renderable_surface_delete(renderable_surface* s) {

  glDeleteBuffers(1 , &s->vertex_vbo);
  glDeleteBuffers(1 , &s->triangle_vbo);
  
  material_delete(s->instance);
  
  free(s);
  
}

void renderable_surface_set_material(renderable_surface* s, material* m) {
  
  s->base = m;
  
}
