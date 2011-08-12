#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asset_manager.h"
#include "geometry.h"

#include "vertex_hashtable.h"

#include "obj_loader.h"

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

model* obj_load_file(char* filename) {

  fflush(stdout);
  
  model* obj_model = malloc(sizeof(model));
  
  char* name = malloc( strlen(filename) +1 );
  strcpy(name, filename);
  
  obj_model->name = name;
  obj_model->num_meshes = 0;
  obj_model->meshes = malloc(sizeof(model*) * 0);
  
  char* contents = asset_load_file(filename);
  
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
  
  return obj_model;
};


model* obj_load_object(char* c) {
  
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

void obj_parse_triangle(char* line, vector3* positions, vector3* normals, vector2* uvs, vertex* v1, vertex* v2, vertex* v3) {
  
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