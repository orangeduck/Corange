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

  int i;
  for(i = 0; i < m->num_meshes; i++) {
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
  
  int i;
  for(i = 0; i < r->num_surfaces; i++) {
    renderable_surface_delete( r->surfaces[i] );
  }
  
  free(r);

}

void renderable_set_material(renderable* r, material* m) {
  int i;
  for(i = 0; i < r->num_surfaces; i++) {
    renderable_surface_set_material(r->surfaces[0], m);
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
  
  material* base;
  
  //printf("mat: |%s|\n", m->material); fflush(stdout);
  
  if( asset_loaded(m->material) ) {
    s->base = asset_get(m->material);
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
  
  int i;
  for(i = 0; i < m->num_verts; i++) {
  
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

static void obj_parse_triangle(char* line, vector3* positions, vector3* normals, vector2* uvs, vertex* v1, vertex* v2, vertex* v3) {
  
  char* end;
  
  int i1 = strtoul(line, &end, 0);
  int i2 = strtoul(end+1, &end, 0);
  int i3 = strtoul(end+1, &end, 0);

  int i4 = strtoul(end+1, &end, 0);
  int i5 = strtoul(end+1, &end, 0);
  int i6 = strtoul(end+1, &end, 0);
  
  int i7 = strtoul(end+1, &end, 0);
  int i8 = strtoul(end+1, &end, 0);
  int i9 = strtoul(end+1, &end, 0);
    
  v1->position = positions[i1-1];
  v1->uvs = uvs[i2-1];
  v1->normal = normals[i3-1];
  
  v2->position = positions[i4-1];
  v2->uvs = uvs[i5-1];
  v2->normal = normals[i6-1];
  
  v3->position = positions[i7-1];
  v3->uvs = uvs[i8-1];
  v3->normal = normals[i9-1];

  /* Set non parsed properties to 0 */
  
  v1->tangent = v3_zero();
  v1->binormal = v3_zero();
  v1->color = v4_zero();
  
  v2->tangent = v3_zero();
  v2->binormal = v3_zero();
  v2->color = v4_zero();
  
  v3->tangent = v3_zero();
  v3->binormal = v3_zero();
  v3->color = v4_zero();
  
}

static model* obj_load_object(char* c) {
  
  /*
     To avoid errors later on, shifting pointer onwards one.
     This is because it probably is currently on an "o" character, which will boot it out of processing.
  */
    
  c++;
  
  /* Allocate a new model */
  model* obj_model = malloc(sizeof(model));
  obj_model->name = "Temp Model Part";
  
  /* Init counters */
  int mesh_count = 0;
  int vert_pos_count = 0;
  int vert_norm_count = 0;
  int vert_uv_count = 0;
  
  /* Maximum 512 submeshes should be enough */
  int* mesh_tri_counts = malloc(sizeof(int) * 512); 
  
  /* Init tri counts to 0 */
  int tc;
  for(tc = 0; tc < 512; tc++) {
    mesh_tri_counts[tc] = 0;
  }
  
  /* Loop Counters */
  int i, j;
  
  /* First run over and count things */
  i = 0;
  while(1) {
    
    if( c[i] == '\0') { break; }
    else if( c[i] == 'o' && c[i-1] == '\n') { break; }
    
    else if( c[i] == 't' && c[i-1] == 'v' && c[i-2] == '\n') { vert_uv_count++; }
    else if( c[i] == 'n' && c[i-1] == 'v' && c[i-2] == '\n') { vert_norm_count++; }
    else if( c[i] == 'v' && c[i-1] == '\n') { vert_pos_count++; }
    else if( c[i] == 'g' && c[i-1] == '\n') { mesh_count++; }
    else if( c[i] == 'f' && c[i-1] == '\n') { mesh_tri_counts[mesh_count-1]++; }
    /* The above counts the number of triangle entries for each mesh */
    /* This is accurate for the allocation of the triangle index struct, but vertex struct should be smaller (verticies on more than one triangle) */
    
    i++;
  }
  
  /* Now we have counts, allocate space for vertex tables */
  vector3* positions = malloc(sizeof(vector3) * vert_pos_count);
  vector3* normals = malloc(sizeof(vector3) * vert_norm_count);
  vector2* uvs = malloc(sizeof(vector2) * vert_uv_count);
  
  /* Allocate space for meshes in model */
  obj_model->meshes = malloc(sizeof(mesh*) * mesh_count);
  obj_model->num_meshes = 0;
  
  /* Allocate a pointer to store working mesh at */
  mesh* current_mesh = NULL;
  
  /* Now we go over file again, parsing line for line */
  /* allocate line 512 characters. */
  char* line = malloc(1024);
  
  /* Variables to index vert tables */
  int v = 0;
  int vn = 0;
  int vt = 0;
  
  i = 0;
  j = 0;

    
    /* So the plan
    
      - Fill Vertex tables
      
      - For triangles:
        - Create new vertex, using pointers to vertex tables.
        - Check if already exists, otherwise add to vertex list.
        - Fill in triangle index using above data.
        
        - Reallocate vertex structure to appropriate size.
    */
  
  vertex_hashtable* hashes = NULL;
  
  while(1) {
  
    /* If end of string or we've reached a new object then exit. */
    if( c[i] == '\0') { break; }
    if( c[i] == '\n' && c[i+1] == 'o') { break; }
    
    /* End of line reached */
    if( c[i] == '\n') {
    
      /* Null terminate line buffer */
      line[j-1] = '\0';
      
      //printf("LINE: %s \n",line);
      
      /* ------------ */
      /* Process Line */
      /* ------------ */
      
        /* Convert to vector, index at +2 to ignore the first 2 characters at beginning of line */
        if ((line[0] == 'v') && (line[1] == 't')) {
          
          vector2 vec = v2_from_string( line+2 );
          uvs[vt] = vec;
          vt++;
        
        } else if ((line[0] == 'v') && (line[1] == 'n')) {
          
          vector3 vec = v3_from_string( line+2 );
          normals[vn] = vec;
          vn++;
        
        } else if (line[0] == 'v') {
          
          vector3 vec = v3_from_string( line+1 );
          positions[v] = vec;
          v++;
        
        } else if (line[0] == 'g') {
          
          /* If we have an existing mesh, attach it to the model */
          if (current_mesh != NULL) {
            model_add_mesh(obj_model, current_mesh);
          }
          
          /* Allocate space for a new one */
          current_mesh = malloc(sizeof(mesh));
          
          char* mesh_name = malloc( strlen(line) - 1 );
          strcpy(mesh_name, line + 2);
          
          current_mesh->name = mesh_name;
          
          /* Default counts to 0, we will use this to index the memory */
          current_mesh->num_triangles = 0;
          current_mesh->num_triangles_3 = 0;
          current_mesh->num_verts = 0;
          
          /* As mentioned above, this is an overestimate (at least for verticies), will be reduced later */ 
          current_mesh->triangles = malloc(sizeof(int) * mesh_tri_counts[obj_model->num_meshes] * 3 );
          current_mesh->verticies = malloc(sizeof(vertex) * mesh_tri_counts[obj_model->num_meshes] * 3);
          
          if(hashes != NULL) {
            vertex_hashtable_delete(hashes);
          }
          
          hashes = vertex_hashtable_new(1000);
          
        } else if (line[0] == 'f') {
          
          vertex v1;
          vertex v2;
          vertex v3;
          
          obj_parse_triangle( line+1, positions, normals, uvs, &v1, &v2, &v3);
          
          int v1_pos = vertex_hashtable_get(hashes, v1);
          if(v1_pos == -1) {
            int pos = mesh_append_vertex(current_mesh, v1);
            vertex_hashtable_set(hashes, v1, pos);
          } else {
            mesh_append_triangle_entry(current_mesh, v1_pos);
          }
          
          int v2_pos = vertex_hashtable_get(hashes, v2);
          if(v2_pos == -1) {
            int pos = mesh_append_vertex(current_mesh, v2);
            vertex_hashtable_set(hashes, v2, pos);
          } else {
            mesh_append_triangle_entry(current_mesh, v2_pos);
          }
          
          int v3_pos = vertex_hashtable_get(hashes, v3);
          if(v3_pos == -1) {
            int pos = mesh_append_vertex(current_mesh, v3);
            vertex_hashtable_set(hashes, v3, pos);
          } else {
            mesh_append_triangle_entry(current_mesh, v3_pos);
          }
          
        } else if ( strstr(line, "usemtl") ) {
          
          char* material_name = malloc( strlen(line) - 5 );
          strcpy(material_name, line + 7 );
          
          current_mesh->material = material_name;
          
        }
      
      /* ---------------- */
      /* End Process Line */
      /* ---------------- */
      
      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      /* Otherwise add character to line buffer */
      line[j] = c[i];
      j++;
    
    }
    
    i++;
  }
  
  free(line);
  
  /* If we have an existing mesh, attach it to the model */
  if (current_mesh != NULL) {
    model_add_mesh(obj_model, current_mesh);
  }
  
  /* Free vertex tables */
  
  vertex_hashtable_delete(hashes);
  
  free(mesh_tri_counts);
  free(positions);
  free(normals);
  free(uvs);
    
  return obj_model;

}

/* Loads contents of file into a model structure

  According to various reseach and the fact I'm not clever enought to do super smart, this seems the best option 
    * o specifies new object, with new vertex list etc
    * # is comment, ignore
    
    * v is vertex position
    * vt is uv
    * vn is normal
    
    * f is face, specified as indicies into the other tables for three verticies
    * g is a new material group (new mesh)
    * usemtl is material name
    
    
    - Load file into buffer.
    - Count Number of verticies
    - Count Number of triangles
    - Ignore multiple objects for now
  
    - Go through and fill arrays.
    - build each sub mesh on it's own
    
    - attach all submeshes to final object.
*/

/* NOTE: Currently not working with multiple objects */

renderable* obj_load_file(char* filename) {
  
  model* obj_model = malloc(sizeof(model));
  
  char* name = malloc( strlen(filename) +1 );
  strcpy(name, filename);
  
  obj_model->name = name;
  obj_model->num_meshes = 0;
  obj_model->meshes = malloc(sizeof(model*) * 0);
  
  char* contents = asset_file_contents(filename);
  
  char* c = contents; // Pointer to beginning of file
  
  while( *c != '\0' ) {
    
    /* If you find an "o" on a new line then parse it as object. */
    if ((*c == 'o') && (*(c-1) == '\n')) {
      
      model* part = obj_load_object(c);
      
      /* Merge loaded model into existing model */
      model_merge_model(obj_model, part);
    }
    c++;
  }
  
  free(contents);
  
  model_generate_tangents(obj_model);
  //model_generate_orthagonal_tangents(obj_model);
  
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
  
  char line[1024];
  
  char* c = asset_file_contents(filename);
  
  int i = 0;
  int j = 0;
  
  int state = state_load_empty;
  
  vertex_hashtable* hashes = vertex_hashtable_new(1024);
  
  vertex_list* vert_list = vertex_list_new();
  int_list* tri_list = int_list_new();
  vertex_weight* weights = malloc(sizeof(vertex_weight) * 10000);
  
  int vert_index = 0;
  
  while(1) {
    if( c[i] == '\0') { break; }
    
    if( c[i] == '\n' ) {
      
      line[j] = '\0';
      
        /* Process line */
        if (state == state_load_empty) {
          
          int version;
          if (sscanf(line, "version %i", &version) > 0) {
            if (version != 1) {
              error("Can't load SMD file %s. Don't know how to load version %i\n", filename, version);
            }
          }
          
          if (strcmp(line, "triangles") == 0) {
            
            state = state_load_triangles;
          }
          
        }
        
        else if (state == state_load_triangles) {
          
          int id, l1_id, l2_id, l3_id;
          int num_links = 0;
          float x, y, z, nx, ny, nz, u, v, l1_amount, l2_amount, l3_amount;
          char links[1024];
          if (sscanf(line, "%i %f %f %f %f %f %f %f %f %i %i %f %i %f %i %f", 
              &id, &x, &y, &z, &nx, &ny, &nz, &u, &v, &num_links, 
              &l1_id, &l1_amount, &l2_id, &l2_amount, &l3_id, &l3_amount) > 0) {
            
            vertex vert;
            /* Swap y and z axis */
            vert.position = v3(x, z, y);
            vert.normal = v3(nx, nz, ny);
            vert.uvs = v2(u, v);
            vert.color = v4_one();
            vert.tangent = v3_zero();
            vert.binormal = v3_zero();
            
            int vert_pos = vertex_hashtable_get(hashes, vert);
            if (vert_pos == -1) {
              vertex_hashtable_set(hashes, vert, vert_index);
              vert_pos = vert_index;
              vertex_list_push_back(vert_list, vert);
              vert_index++;
            }
            
            if (num_links > 3) {
              warning("Loading file %s. More than 3 bones rigged to vertex. Ignoring other bones", filename);
              num_links = 3;
            }
            
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
            } else if (num_links == 0) {
              warning("Loading file %s. Unrigged vertex!", filename);
              vw.bone_ids[0] = 0; vw.bone_ids[1] = 0; vw.bone_ids[2] = 0;
              vw.bone_weights[0] = 1; vw.bone_weights[1] = 0; vw.bone_weights[2] = 0;
            }
            weights[vert_pos] = vw;
            
            int_list_push_back(tri_list, vert_pos);
            
          }
          
          if (strcmp(line, "end") == 0) {
            state = state_load_empty;
          }
          
        }
        
        /* End Process line */
      
      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      line[j] = c[i];
      j++;
    
    }
    
    i++;
  }

  free(c);
  
  mesh* smd_mesh = malloc(sizeof(mesh));
  smd_mesh->name = malloc(strlen("pirate_mesh") + 1);
  strcpy(smd_mesh->name, "pirate_mesh");
  smd_mesh->material = malloc(strlen("pirate_mat") + 1);
  strcpy(smd_mesh->material, "pirate_mat");
  smd_mesh->num_verts = vert_list->num_items;
  smd_mesh->num_triangles = tri_list->num_items / 3;
  smd_mesh->num_triangles_3 = tri_list->num_items;
  smd_mesh->verticies = malloc(sizeof(vertex) * smd_mesh->num_verts);
  smd_mesh->triangles = malloc(sizeof(int) * smd_mesh->num_triangles_3);
  
  for(i = 0; i < smd_mesh->num_verts; i++) {
    smd_mesh->verticies[i] = vertex_list_get(vert_list, i);
  }
  
  for(i = 0; i < smd_mesh->num_triangles_3; i++) {
    smd_mesh->triangles[i] = int_list_get(tri_list, i);
  }
  
  vertex_hashtable_delete(hashes);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  free(weights);
  
  mesh_generate_tangents(smd_mesh);
  
  renderable* renderable = renderable_new();
  renderable_add_mesh_rigged(renderable, smd_mesh, weights);
  
  mesh_delete(smd_mesh);
  
  return renderable;
}

