#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "vector.h"
#include "asset_manager.h"
#include "vertex_hashtable.h"

#include "renderable.h"

static void renderable_add_mesh(renderable* r, mesh* m) {
  
  renderable_surface* surface = renderable_surface_new(m);
  
  r->num_surfaces++;
  r->surfaces = realloc(r->surfaces, sizeof(renderable_surface*) *  r->num_surfaces);
  r->surfaces[r->num_surfaces-1] = surface;
  
}

static void renderable_add_model(renderable* r, model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    renderable_add_mesh(r, m->meshes[i]);
  }
  
}

renderable* renderable_new() {
  
  renderable* r = malloc(sizeof(renderable));
  
  r->num_surfaces = 0;
  r->surfaces = NULL;
    
  return r;

}

void renderable_delete(renderable* r) {
  
  for(int i = 0; i < r->num_surfaces; i++) {
    renderable_surface_delete( r->surfaces[i] );
  }
  
  free(r);

}

void renderable_set_material(renderable* r, material* m) {
  for(int i = 0; i < r->num_surfaces; i++) {
    renderable_surface_set_material(r->surfaces[i], m);
  }
}

void renderable_set_multi_material(renderable* r, multi_material* mmat) {
  
  int min_range = min(r->num_surfaces, mmat->num_materials);
  
  for(int i = 0; i < min_range; i++) {
    renderable_surface_set_material(r->surfaces[i], mmat->materials[i]);
  }
  
}

renderable_surface* renderable_surface_new(mesh* m) {

  renderable_surface* s = malloc(sizeof(renderable_surface));

  glGenBuffers(1, &s->vertex_vbo);
  glGenBuffers(1, &s->triangle_vbo);
  
  s->is_rigged = 0;
  s->num_verticies = m->num_verts;
  s->num_triangles = m->num_triangles;
  
  /* Position Normal Tangent Binormal Uvs Color      */
  /* 3        3      3       3        2   4     = 18 */
  float* vb_data = malloc(sizeof(float) * m->num_verts * 18);
  
  for(int i = 0; i < m->num_verts; i++) {
  
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
  
  material* base;
  
  if( asset_loaded(m->material) ) {
    s->base = asset_get(m->material);
  } else {
    
    s->base = asset_load_get("$CORANGE/resources/basic.mat");
  }
  
  return s;
}

renderable_surface* renderable_surface_new_rigged(mesh* m, vertex_weight* weights) {

  renderable_surface* s = malloc(sizeof(renderable_surface));

  glGenBuffers(1, &s->vertex_vbo);
  glGenBuffers(1, &s->triangle_vbo);
  
  s->is_rigged = 1;
  s->num_verticies = m->num_verts;
  s->num_triangles = m->num_triangles;
  
  /* Position Normal Tangent Binormal Uvs Color WeightIds WeightAmounts      */
  /* 3        3      3       3        2   4     3         3             = 24 */
  float* vb_data = malloc(sizeof(float) * m->num_verts * 24);
  
  for(int i = 0; i < m->num_verts; i++) {
  
    vector3 pos = m->verticies[i].position;
    vector3 norm = m->verticies[i].normal;
    vector3 tang = m->verticies[i].tangent;
    vector3 bino = m->verticies[i].binormal;
    vector2 uvs = m->verticies[i].uvs;
    vector4 col = m->verticies[i].color;
    
    vb_data[(i*24)+0] = pos.x;
    vb_data[(i*24)+1] = pos.y;
    vb_data[(i*24)+2] = pos.z;
    
    vb_data[(i*24)+3] = norm.x;
    vb_data[(i*24)+4] = norm.y;
    vb_data[(i*24)+5] = norm.z;
    
    vb_data[(i*24)+6] = tang.x;
    vb_data[(i*24)+7] = tang.y;
    vb_data[(i*24)+8] = tang.z;
    
    vb_data[(i*24)+9] = bino.x;
    vb_data[(i*24)+10] = bino.y;
    vb_data[(i*24)+11] = bino.z;
    
    vb_data[(i*24)+12] = uvs.x;
    vb_data[(i*24)+13] = uvs.y;
    
    vb_data[(i*24)+14] = col.w;
    vb_data[(i*24)+15] = col.x;
    vb_data[(i*24)+16] = col.y;
    vb_data[(i*24)+17] = col.z;
  
    vb_data[(i*24)+18] = (float)weights[i].bone_ids[0];
    vb_data[(i*24)+19] = (float)weights[i].bone_ids[1];
    vb_data[(i*24)+20] = (float)weights[i].bone_ids[2];
    
    vb_data[(i*24)+21] = weights[i].bone_weights[0];
    vb_data[(i*24)+22] = weights[i].bone_weights[1];
    vb_data[(i*24)+23] = weights[i].bone_weights[2];
  
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s->num_verticies * 24, vb_data, GL_STATIC_DRAW);
  
  free(vb_data);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * s->num_triangles * 3, m->triangles, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  material* base;
  
  if( asset_loaded(m->material) ) {
    s->base = asset_get(m->material);
  } else {
    s->base = asset_load_get("$CORANGE/resources/basic.mat");
  }
  
  return s;

}

void renderable_surface_delete(renderable_surface* s) {

  glDeleteBuffers(1 , &s->vertex_vbo);
  glDeleteBuffers(1 , &s->triangle_vbo);
  
  free(s);
  
}

void renderable_surface_set_material(renderable_surface* s, material* m) {
  s->base = m;
}

renderable* obj_load_file(char* filename) {
  
  model* obj_model = malloc(sizeof(model));
  
  obj_model->name = malloc( strlen(filename) +1 );
  strcpy(obj_model->name, filename);
  
  obj_model->num_meshes = 0;
  obj_model->meshes = malloc(sizeof(mesh*) * 0);
  
  mesh* active_mesh = NULL;
  
  vertex_list* vert_data = vertex_list_new_blocksize(1024);
  vertex_list* vert_list = vertex_list_new_blocksize(1024);
  int_list* tri_list = int_list_new_blocksize(1024);
  vertex_hashtable* vert_hashes = vertex_hashtable_new(4096);
  
  int num_pos, num_norm, num_tex;
  num_pos = num_norm = num_tex = 0;
  
  int vert_index = 0;
  
  bool has_normal_data = false;
  bool has_texcoord_data = false;
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    char comment[512];
    char matlib[512];
    char object[512];
    char group[512];
    char material[512];
    float px, py, pz, tx, ty, nx, ny, nz;
    int smoothing_group;
    int pi1, ti1, ni1, pi2, ti2, ni2, pi3, ti3, ni3;
    
    if (sscanf(line, "# %512s", comment) == 1) {
      /* Comment, do nothing */
    }
    
    else if (sscanf(line, "mtllib %512s", matlib) == 1) {
      /* Material library, do nothing */
    }
    
    else if (sscanf(line, "o %512s", object) == 1) {
      if (active_mesh != NULL) {
        active_mesh->name = realloc(active_mesh->name, strlen(object) + 1);
        strcpy(active_mesh->name, object);
      }
    }
    
    else if (sscanf(line, "v %f %f %f", &px, &py, &pz) == 3) {
    
      while(vert_data->num_items <= num_pos) { vertex_list_push_back(vert_data, vertex_new()); }
      vertex vert = vertex_list_get(vert_data, num_pos);
      vert.position = v3(px, py, pz);
      vertex_list_set(vert_data, num_pos, vert);
      num_pos++;
    }
    
    else if (sscanf(line, "vt %f %f", &tx, &ty) == 2) {
    
      while(vert_data->num_items <= num_tex) { vertex_list_push_back(vert_data, vertex_new()); }
      vertex vert = vertex_list_get(vert_data, num_tex);
      vert.uvs = v2(tx, ty);
      vertex_list_set(vert_data, num_tex, vert);
      num_tex++;
    }
    
    else if (sscanf(line, "vn %f %f %f", &nx, &ny, &nz) == 3) {
    
      while(vert_data->num_items <= num_norm) { vertex_list_push_back(vert_data, vertex_new()); }
      vertex vert = vertex_list_get(vert_data, num_norm);
      vert.normal = v3(nx, ny, nz);
      vertex_list_set(vert_data, num_norm, vert);
      num_norm++;
    }
    
    else if (sscanf(line, "g %512s", group) == 1) {
        
      if (active_mesh != NULL) {
      
        active_mesh->num_verts = vert_index;
        active_mesh->num_triangles = tri_list->num_items / 3;
        active_mesh->num_triangles_3 = tri_list->num_items;
        
        active_mesh->verticies = malloc(sizeof(vertex) * active_mesh->num_verts);
        for(int i = 0; i < active_mesh->num_verts; i++) {
          active_mesh->verticies[i] = vertex_list_get(vert_list, i);
        }
        
        active_mesh->triangles = malloc(sizeof(int) * active_mesh->num_triangles_3);
        for(int i = 0; i < active_mesh->num_triangles_3; i++) {
          active_mesh->triangles[i] = int_list_get(tri_list, i);
        }
      
        obj_model->num_meshes++;
        obj_model->meshes = realloc(obj_model->meshes, sizeof(mesh*) * obj_model->num_meshes);
        obj_model->meshes[obj_model->num_meshes-1] = active_mesh;
        
      }
      
      vert_index = 0;
      
      vertex_hashtable_delete(vert_hashes);
      vertex_list_delete(vert_list);
      int_list_delete(tri_list);
      
      vert_list = vertex_list_new_blocksize(1024);
      tri_list = int_list_new_blocksize(1024);
      vert_hashes = vertex_hashtable_new(4096);
      
      active_mesh = malloc(sizeof(mesh));
      
      active_mesh->material = malloc(strlen("$CORANGE/resources/basic.mat") + 1);
      strcpy(active_mesh->material, "$CORANGE/resources/basic.mat");
      
      active_mesh->name = malloc(strlen("mesh1") + 1);
      strcpy(active_mesh->name, "mesh1");
      
    }
    
    else if (sscanf(line, "usemtl %512s", material) == 1) {
      if (active_mesh != NULL) {
        active_mesh->material = realloc(active_mesh->material, strlen(material) + 1);
        strcpy(active_mesh->material, material);
      }
    }
    
    else if (sscanf(line, "s %i", &smoothing_group) == 1) {
      /* Smoothing group, do nothing */
    }
    
    else if (sscanf(line, "f %i/%i/%i %i/%i/%i %i/%i/%i", &pi1, &ti1, &ni1, &pi2, &ti2, &ni2, &pi3, &ti3, &ni3) == 9) {
      has_normal_data = true;
      has_texcoord_data = true;
      
      /* OBJ file indicies start from one, have to subtract one */
      pi1--; ti1--; ni1--; pi2--; ti2--; ni2--; pi3--; ti3--; ni3--;
      
      vertex v1, v2, v3;
      v1.position = vertex_list_get(vert_data, pi1).position;
      v1.uvs = vertex_list_get(vert_data, ti1).uvs;
      v1.normal = vertex_list_get(vert_data, ni1).normal;
      
      v2.position = vertex_list_get(vert_data, pi2).position;
      v2.uvs = vertex_list_get(vert_data, ti2).uvs;
      v2.normal = vertex_list_get(vert_data, ni2).normal;
      
      v3.position = vertex_list_get(vert_data, pi3).position;
      v3.uvs = vertex_list_get(vert_data, ti3).uvs;
      v3.normal = vertex_list_get(vert_data, ni3).normal;
      
      int v1_id = vertex_hashtable_get(vert_hashes, v1);
      if ( v1_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v1, vert_index);
        vertex_list_push_back(vert_list, v1);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v1_id);
      }
      
      int v2_id = vertex_hashtable_get(vert_hashes, v2);
      if ( v2_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v2, vert_index);
        vertex_list_push_back(vert_list, v2);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v2_id);
      }
      
      int v3_id = vertex_hashtable_get(vert_hashes, v3);
      if ( v3_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v3, vert_index);
        vertex_list_push_back(vert_list, v3);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v3_id);
      }
      
    }
    
    else if (sscanf(line, "f %i//%i %i//%i %i//%i", &pi1, &ni1, &pi2, &ni2, &pi3, &ni3) == 6) {
      has_normal_data = true;
      has_texcoord_data = false;
      
      /* OBJ file indicies start from one, have to subtract one */
      pi1--; ni1--; pi2--; ni2--; pi3--; ni3--;
      
      vertex v1, v2, v3;
      v1.position = vertex_list_get(vert_data, pi1).position;
      v1.uvs = v2_zero();
      v1.normal = vertex_list_get(vert_data, ni1).normal;
      
      v2.position = vertex_list_get(vert_data, pi2).position;
      v2.uvs = v2_zero();
      v2.normal = vertex_list_get(vert_data, ni2).normal;
      
      v3.position = vertex_list_get(vert_data, pi3).position;
      v3.uvs = v2_zero();
      v3.normal = vertex_list_get(vert_data, ni3).normal;
      
      int v1_id = vertex_hashtable_get(vert_hashes, v1);
      if ( v1_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v1, vert_index);
        vertex_list_push_back(vert_list, v1);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v1_id);
      }
      
      int v2_id = vertex_hashtable_get(vert_hashes, v2);
      if ( v2_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v2, vert_index);
        vertex_list_push_back(vert_list, v2);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v2_id);
      }
      
      int v3_id = vertex_hashtable_get(vert_hashes, v3);
      if ( v3_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v3, vert_index);
        vertex_list_push_back(vert_list, v3);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v3_id);
      }
      
    }
    
    else if (sscanf(line, "f %i/%i %i/%i %i/%i", &pi1, &ti1, &pi2, &ti2, &pi3, &ti3) == 6) {
      has_normal_data = false;
      has_texcoord_data = true;
      
      /* OBJ file indicies start from one, have to subtract one */
      pi1--; ti1--; pi2--; ti2--; pi3--; ti3--;
      
      vertex v1, v2, v3;
      v1.position = vertex_list_get(vert_data, pi1).position;
      v1.uvs = vertex_list_get(vert_data, ti1).uvs;
      v1.normal = v3_zero();
      
      v2.position = vertex_list_get(vert_data, pi2).position;
      v2.uvs = vertex_list_get(vert_data, ti2).uvs;
      v2.normal = v3_zero();
      
      v3.position = vertex_list_get(vert_data, pi3).position;
      v3.uvs = vertex_list_get(vert_data, ti3).uvs;
      v3.normal = v3_zero();
      
      int v1_id = vertex_hashtable_get(vert_hashes, v1);
      if ( v1_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v1, vert_index);
        vertex_list_push_back(vert_list, v1);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v1_id);
      }
      
      int v2_id = vertex_hashtable_get(vert_hashes, v2);
      if ( v2_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v2, vert_index);
        vertex_list_push_back(vert_list, v2);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v2_id);
      }
      
      int v3_id = vertex_hashtable_get(vert_hashes, v3);
      if ( v3_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v3, vert_index);
        vertex_list_push_back(vert_list, v3);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v3_id);
      }
      
    }
    
    else if (sscanf(line, "f %i %i %i", &pi1, &pi2, &pi3) == 3) {
      has_normal_data = false;
      has_texcoord_data = false;
      
      /* OBJ file indicies start from one, have to subtract one */
      pi1--; pi2--; pi3--;
      
      vertex v1, v2, v3;
      v1.position = vertex_list_get(vert_data, pi1).position;
      v1.uvs = v2_zero();
      v1.normal = v3_zero();
      
      v2.position = vertex_list_get(vert_data, pi2).position;
      v2.uvs = v2_zero();
      v2.normal = v3_zero();
      
      v3.position = vertex_list_get(vert_data, pi3).position;
      v3.uvs = v2_zero();
      v3.normal = v3_zero();
      
      int v1_id = vertex_hashtable_get(vert_hashes, v1);
      if ( v1_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v1, vert_index);
        vertex_list_push_back(vert_list, v1);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v1_id);
      }
      
      int v2_id = vertex_hashtable_get(vert_hashes, v2);
      if ( v2_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v2, vert_index);
        vertex_list_push_back(vert_list, v2);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v2_id);
      }
      
      int v3_id = vertex_hashtable_get(vert_hashes, v3);
      if ( v3_id == -1 ) {
        vertex_hashtable_set(vert_hashes, v3, vert_index);
        vertex_list_push_back(vert_list, v3);
        int_list_push_back(tri_list, vert_index);
        vert_index++;
      } else {
        int_list_push_back(tri_list, v3_id);
      }
      
    }
    
  }

  SDL_RWclose(file);
  
  active_mesh->num_verts = vert_index;
  active_mesh->num_triangles = tri_list->num_items / 3;
  active_mesh->num_triangles_3 = tri_list->num_items;
  
  active_mesh->verticies = malloc(sizeof(vertex) * active_mesh->num_verts);
  for(int i = 0; i < active_mesh->num_verts; i++) {
    active_mesh->verticies[i] = vertex_list_get(vert_list, i);
  }
  
  active_mesh->triangles = malloc(sizeof(int) * active_mesh->num_triangles_3);
  for(int i = 0; i < active_mesh->num_triangles_3; i++) {
    active_mesh->triangles[i] = int_list_get(tri_list, i);
  }
  
  obj_model->num_meshes++;
  obj_model->meshes = realloc(obj_model->meshes, sizeof(mesh*) * obj_model->num_meshes);
  obj_model->meshes[obj_model->num_meshes-1] = active_mesh;
  
  vertex_hashtable_delete(vert_hashes);
  vertex_list_delete(vert_data);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  
  if (!has_normal_data) {
    model_generate_normals(obj_model);
  }
  
  if (!has_texcoord_data) {
    model_generate_texcoords_cylinder(obj_model);
  }
  
  model_generate_tangents(obj_model);
  
  renderable* renderable = renderable_new();
  renderable_add_model(renderable, obj_model);
  
  model_delete(obj_model);
  
  return renderable;
}

static void renderable_add_mesh_rigged(renderable* r, mesh* m, vertex_weight* weights) {
  
  renderable_surface* surface = renderable_surface_new_rigged(m, weights);
  
  r->num_surfaces++;
  r->surfaces = realloc(r->surfaces, sizeof(renderable_surface*) *  r->num_surfaces);
  r->surfaces[r->num_surfaces-1] = surface;
  
}

static int state_load_empty = 0;
static int state_load_triangles = 1;

renderable* smd_load_file(char* filename) {
  
  int state = state_load_empty;
  
  vertex_hashtable* hashes = vertex_hashtable_new(1024);
  vertex_list* vert_list = vertex_list_new();
  int_list* tri_list = int_list_new();
  
  int allocated_weights = 1024;
  vertex_weight* weights = malloc(sizeof(vertex_weight) * allocated_weights);
  
  int vert_index = 0;
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (state == state_load_empty) {
      
      int version;
      if (sscanf(line, "version %i", &version) > 0) {
        if (version != 1) {
          error("Can't load SMD file %s. Don't know how to load version %i\n", filename, version);
        }
      }
      
      if (strstr(line, "triangles")) {
        state = state_load_triangles;
      }
    }
    
    else if (state == state_load_triangles) {
      
      int id, l1_id, l2_id, l3_id;
      int num_links = 0;
      float x, y, z, nx, ny, nz, u, v, l1_amount, l2_amount, l3_amount;
      if (sscanf(line, "%i %f %f %f %f %f %f %f %f %i %i %f %i %f %i %f", 
          &id, &x, &y, &z, &nx, &ny, &nz, &u, &v, &num_links, 
          &l1_id, &l1_amount, &l2_id, &l2_amount, &l3_id, &l3_amount) > 0) {
        
        if (num_links > 3) {
          warning("Loading file %s. More than 3 bones rigged to vertex. Ignoring other bones", filename);
          num_links = 3;
        }
        
        if (num_links == 0) {
          warning("Loading file %s. Vertex has no direct bone links", filename);
          num_links = 1;
          l1_id = id;
        }
        
        vertex vert;
        /* Swap y and z axis */
        vert.position = v3(x, z, y);
        vert.normal = v3(nx, nz, ny);
        vert.uvs = v2(u, v);
        vert.color = v4_one();
        vert.tangent = v3_zero();
        vert.binormal = v3_zero();
        
        int vert_pos = vertex_hashtable_get(hashes, vert);
        
        /* Not already in hashtable */
        if (vert_pos == -1) {
          vertex_hashtable_set(hashes, vert, vert_index);
          vert_pos = vert_index;
          vertex_list_push_back(vert_list, vert);
          
          vertex_weight vw;
          if (num_links == 3) {
            vw.bone_ids[0] = l1_id; vw.bone_ids[1] = l2_id; vw.bone_ids[2] = l3_id;
            vw.bone_weights[0] = l1_amount; vw.bone_weights[1] = l2_amount; vw.bone_weights[2] = l3_amount;
          } else if (num_links == 2) {
            vw.bone_ids[0] = l1_id; vw.bone_ids[1] = l2_id; vw.bone_ids[2] = 0;
            vw.bone_weights[0] = l1_amount; vw.bone_weights[1] = l2_amount; vw.bone_weights[2] = 0;
          } else if (num_links == 1) {
            vw.bone_ids[0] = l1_id; vw.bone_ids[1] = 0; vw.bone_ids[2] = 0;
            vw.bone_weights[0] = 1; vw.bone_weights[1] = 0; vw.bone_weights[2] = 0;
          } else {
            warning("Loading file %s. Unrigged vertex!", filename);
            vw.bone_ids[0] = 0; vw.bone_ids[1] = 0; vw.bone_ids[2] = 0;
            vw.bone_weights[0] = 1; vw.bone_weights[1] = 0; vw.bone_weights[2] = 0;
          }
          
          while(vert_pos >= allocated_weights) {
            allocated_weights = allocated_weights * 2;
            weights = realloc(weights, sizeof(vertex_weight) * allocated_weights);
          }

          weights[vert_pos] = vw;
          
          vert_index++;
        }
        
        int_list_push_back(tri_list, vert_pos);
        
      }
      
    }
    
    if (strstr(line, "end")) {
      state = state_load_empty;
    }
    
  }
  
  SDL_RWclose(file);
  
  mesh* smd_mesh = malloc(sizeof(mesh));
  smd_mesh->name = malloc(strlen(filename) + 1);
  strcpy(smd_mesh->name, filename);
  
  smd_mesh->material = malloc(strlen("$CORANGE/resources/basic_animated.mat") + 1);
  strcpy(smd_mesh->material, "$CORANGE/resources/basic_animated.mat");
  
  smd_mesh->num_verts = vert_list->num_items;
  smd_mesh->num_triangles = tri_list->num_items / 3;
  smd_mesh->num_triangles_3 = tri_list->num_items;
  
  smd_mesh->verticies = malloc(sizeof(vertex) * smd_mesh->num_verts);
  smd_mesh->triangles = malloc(sizeof(int) * smd_mesh->num_triangles_3);
  
  for(int i = 0; i < smd_mesh->num_verts; i++) {
    smd_mesh->verticies[i] = vertex_list_get(vert_list, i);
  }
  
  for(int i = 0; i < smd_mesh->num_triangles_3; i++) {
    smd_mesh->triangles[i] = int_list_get(tri_list, i);
  }
  
  mesh_generate_tangents(smd_mesh);
  
  renderable* renderable = renderable_new();
  renderable_add_mesh_rigged(renderable, smd_mesh, weights);
  
  vertex_hashtable_delete(hashes);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  mesh_delete(smd_mesh);
  free(weights);
  
  return renderable;
}

