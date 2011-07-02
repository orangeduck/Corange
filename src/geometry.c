#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vector.h"
#include "geometry.h"

int vertex_equal(vertex v1, vertex v2) {
  
  if(!v3_equ(v1.position, v2.position)) { return 0; }
  if(!v3_equ(v1.normal, v2.normal)) { return 0; }
  if(!v2_equ(v1.uvs, v2.uvs)) { return 0; }
  
  return 1;  
}

void vertex_print(vertex v) {

  printf("V(Position: "); v3_print(v.position);
  printf(", Normal: "); v3_print(v.normal);
  printf(", Tangent: "); v3_print(v.tangent);
  printf(", Binormal: "); v3_print(v.binormal);
  printf(", Color: "); v4_print(v.color);
  printf(", Uvs: "); v2_print(v.uvs);
  printf(")");
  
}

int mesh_contains_vert(mesh* m, vertex v, int* position) {
  
  int i = 0;  
  for (i = 0; i < m->num_verts; i++) {
    if (vertex_equal(v, m->verticies[i])) { 
      *position = i;
      return 1;
      }
  }
  
  return 0;
}

static int pos;
void mesh_add_vertex(mesh* m, vertex v) {
  
  /* If mesh already has vertex in list, then point triangle toward this one */
  if ( mesh_contains_vert(m, v, &pos) ) {  
  
    m->triangles[m->num_triangles_3] = pos;
    m->num_triangles_3++;
  
  /* Otherwise add to vertex list, point triangle toward new entry */
  } else {  
  
    m->verticies[m->num_verts] = v;
    m->triangles[m->num_triangles_3] = m->num_verts;
    
    m->num_triangles_3++;
    m->num_verts++;
    
  }

}

int mesh_append_vertex(mesh* m, vertex v) {

    m->verticies[m->num_verts] = v;
    m->triangles[m->num_triangles_3] = m->num_verts;
    
    m->num_triangles_3++;
    m->num_verts++;

    return (m->num_verts - 1);
}

int mesh_append_triangle_entry(mesh* m, int pos) {

    m->triangles[m->num_triangles_3] = pos;
    m->num_triangles_3++;
    
    return (m->num_triangles_3 - 1);
} 

void mesh_print(mesh* m) {
  
  int i;
  
  printf("Mesh Name: %s\n", m->name);
  printf("Material Name: %s\n", m->material);
  printf("Num Verts: %i\n", m->num_verts);
  printf("Num Tris: %i\n", m->num_triangles);
  for(i=0; i < m->num_verts; i++) {
    vertex_print(m->verticies[i]); printf("\n");
  }
  printf("Triangle Indicies:");
  for(i=0; i < m->num_triangles_3; i++) {
    printf("%i ", m->triangles[i]);
  }
  printf("\n");
  
}

void mesh_delete(mesh* m) {
  
  free(m->name);
  free(m->material);
  free(m->verticies);
  free(m->triangles);
  
  free(m);

}

void mesh_generate_tangents(mesh* m) {
  
  int i;
  
  /* Clear all tangents to 0,0,0 */
  
  for(i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_zero();
    m->verticies[i].binormal = v3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  
  i = 0;
  while( i < m->num_triangles_3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vector3 face_tangent = triangle_tangent(v1, v2, v3);
    vector3 face_binormal = triangle_binormal(v1, v2, v3);
    
    v1.tangent = v3_add(face_tangent, v1.tangent);
    v2.tangent = v3_add(face_tangent, v2.tangent);
    v3.tangent = v3_add(face_tangent, v3.tangent);
    
    v1.binormal = v3_add(face_binormal, v1.binormal);
    v2.binormal = v3_add(face_binormal, v2.binormal);
    v3.binormal = v3_add(face_binormal, v3.binormal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  
  /* normalize all tangents */
  
  for(i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = v3_normalize( m->verticies[i].binormal );
  }
  
}

void mesh_generate_orthagonal_tangents(mesh* m) {
  
  int i;
  
  /* Clear all tangents to 0,0,0 */
  
  for(i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_zero();
    m->verticies[i].binormal = v3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  
  i = 0;
  while( i < m->num_triangles_3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vector3 face_normal = triangle_normal(v1, v2, v3);    
    vector3 face_binormal_temp = triangle_binormal(v1, v2, v3);
    
    vector3 face_tangent = v3_normalize( v3_cross(face_binormal_temp, face_normal) );
    vector3 face_binormal = v3_normalize( v3_cross(face_tangent, face_normal) );
    
    v1.tangent = v3_add(face_tangent, v1.tangent);
    v2.tangent = v3_add(face_tangent, v2.tangent);
    v3.tangent = v3_add(face_tangent, v3.tangent);
    
    v1.binormal = v3_add(face_binormal, v1.binormal);
    v2.binormal = v3_add(face_binormal, v2.binormal);
    v3.binormal = v3_add(face_binormal, v3.binormal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  
  /* normalize all tangents */
  
  for(i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = v3_normalize( m->verticies[i].binormal );
  }
  
}

float mesh_surface_area(mesh* m) {
  
  float total = 0.0;
  
  int i = 0;
  while( i < m->num_triangles_3) {
  
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];

    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    total += triangle_area(v1, v2, v3);
    
    i = i + 3;
  }
  
  return total;
  
}

void model_add_mesh(model* main_model, mesh* sub_mesh) {
          
  /* Re fit the vertex and triangle memory sizes */ 
  sub_mesh->verticies = realloc(sub_mesh->verticies, sizeof(vertex) * sub_mesh->num_verts);
  sub_mesh->triangles = realloc(sub_mesh->triangles, sizeof(int) * sub_mesh->num_triangles_3);
  
  /* Calculate triangle count */
  sub_mesh->num_triangles = sub_mesh->num_triangles_3 / 3;
  
  /* Attach to model */  
  main_model->meshes[main_model->num_meshes] = sub_mesh;
  main_model->num_meshes++;
  
}

void model_merge_model(model* m1, model* m2) {

  int total_num_meshes = m1->num_meshes + m2->num_meshes;
  
  /* expand mesh list to allow for m2 meshes */
  m1->meshes = realloc(m1->meshes, sizeof(mesh*) * total_num_meshes);
  
  /* Copy pointers from m2 into the m1 mesh list */
  int i;
  for(i = m1->num_meshes; i < total_num_meshes;i++) {
    m1->meshes[i] = m2->meshes[i - m1->num_meshes];
  }
  
  m1->num_meshes = total_num_meshes;

  free(m2->name);
  free(m2);
}

void model_print(model* m) {
  int i;
  for(i=0; i<m->num_meshes; i++) {
    mesh_print( m->meshes[i] );
  }
}

void model_delete(model* m) {
  
  int i;
  for(i=0; i<m->num_meshes; i++) {
    mesh_delete( m->meshes[i] );
  }
  
  free(m->name);
  free(m);
  
}

void model_generate_tangents(model* m) {

  int i;
  for(i=0; i<m->num_meshes; i++) {
    mesh_generate_tangents( m->meshes[i] );
  }
  
}

void model_generate_orthagonal_tangents(model* m) {

  int i;
  for(i=0; i<m->num_meshes; i++) {
    mesh_generate_orthagonal_tangents( m->meshes[i] );
  }

}

float model_surface_area(model* m) {
  float total = 0.0f;
  int i;
  for(i=0; i<m->num_meshes; i++) {
    total += mesh_surface_area( m->meshes[i] );
  }
  
  return total;
}

render_mesh* to_render_mesh(mesh* old_mesh){
  
  render_mesh* new_mesh = malloc(sizeof(render_mesh));
  new_mesh->name = malloc(strlen(old_mesh->name) + 1);
  strcpy(new_mesh->name, old_mesh->name);
  
  new_mesh->material = malloc(strlen(old_mesh->material) + 1);
  strcpy(new_mesh->material, old_mesh->material);
  
  new_mesh->num_verts = old_mesh->num_verts;
  new_mesh->num_triangles = old_mesh->num_triangles;
  new_mesh->num_triangles_3 = old_mesh->num_triangles_3;
  
  new_mesh->vertex_positions = malloc(sizeof(float) * old_mesh->num_verts * 3);
  new_mesh->vertex_normals = malloc(sizeof(float) * old_mesh->num_verts * 3);
  new_mesh->vertex_tangents = malloc(sizeof(float) * old_mesh->num_verts * 3);
  new_mesh->vertex_binormals = malloc(sizeof(float) * old_mesh->num_verts * 3);
  
  new_mesh->vertex_colors = malloc(sizeof(float) * old_mesh->num_verts * 4);
  new_mesh->vertex_uvs = malloc(sizeof(float) * old_mesh->num_verts * 2);
  
  new_mesh->triangles = malloc(sizeof(int) * old_mesh->num_triangles_3);
  
  /* Copy vertex data */
  vertex vert;
  
  int j, j_2, j_3, j_4;
  for(j=0; j < old_mesh->num_verts; j++) {
  
    vert = old_mesh->verticies[j];
    
    j_4 = j*4;
    j_3 = j*3;
    j_2 = j*2;
    
    new_mesh->vertex_positions[j_3] = vert.position.x;
    new_mesh->vertex_positions[j_3+1] = vert.position.y;
    new_mesh->vertex_positions[j_3+2] = vert.position.z;
    
    new_mesh->vertex_normals[j_3] = vert.normal.x;
    new_mesh->vertex_normals[j_3+1] = vert.normal.y;
    new_mesh->vertex_normals[j_3+2] = vert.normal.z;
    
    new_mesh->vertex_tangents[j_3] = vert.tangent.x;
    new_mesh->vertex_tangents[j_3+1] = vert.tangent.y;
    new_mesh->vertex_tangents[j_3+2] = vert.tangent.z;
    
    new_mesh->vertex_binormals[j_3] = vert.binormal.x;
    new_mesh->vertex_binormals[j_3+1] = vert.binormal.y;
    new_mesh->vertex_binormals[j_3+2] = vert.binormal.z;
    
    new_mesh->vertex_colors[j_4] = vert.color.w;
    new_mesh->vertex_colors[j_4+1] = vert.color.x;
    new_mesh->vertex_colors[j_4+2] = vert.color.y;
    new_mesh->vertex_colors[j_4+3] = vert.color.z;
    
    new_mesh->vertex_uvs[j_2] = vert.uvs.x;
    new_mesh->vertex_uvs[j_2+1] = vert.uvs.y;
    
  }
  
  /* Copy Triangle Data */
  int k;
  for(k=0; k < old_mesh->num_triangles_3; k++) {
    new_mesh->triangles[k] = old_mesh->triangles[k];
  }
  
  return new_mesh;

}

void render_mesh_print(render_mesh* m) {
  
  int i;
  
  printf("Mesh Name: %s\n", m->name);
  printf("Material Name: %s\n", m->material);
  printf("Num Verts: %i\n", m->num_verts);
  printf("Num Tris: %i\n", m->num_triangles);
  printf("Vertex Positions:");
  for(i=0; i < m->num_verts * 3; i++) {
    printf("%4.2f ", m->vertex_positions[i]);
  }
  printf("\n");
  printf("Vertex Normals:");
  for(i=0; i < m->num_verts * 3; i++) {
    printf("%4.2f ", m->vertex_normals[i]);
  }
  printf("\n");
  printf("Vertex Tangents:");
  for(i=0; i < m->num_verts * 3; i++) {
    printf("%4.2f ", m->vertex_tangents[i]);
  }
  printf("\n");
  printf("Vertex Binormals:");
  for(i=0; i < m->num_verts * 3; i++) {
    printf("%4.2f ", m->vertex_binormals[i]);
  }
  printf("\n");
  printf("Vertex Uvs:");
  for(i=0; i < m->num_verts * 2; i++) {
    printf("%4.2f ", m->vertex_uvs[i]);
  }
  printf("\n");
  printf("Vertex Colors:");
  for(i=0; i < m->num_verts * 4; i++) {
    printf("%4.2f ", m->vertex_colors[i]);
  }
  printf("\n");
  printf("Triangle Indicies:");
  for(i=0; i < m->num_triangles_3; i++) {
    printf("%i ", m->triangles[i]);
  }
  
  printf("\n");
  
};

void render_mesh_delete(render_mesh* m) {

  free(m->name);
  free(m->material);
  
  free(m->vertex_positions);
  free(m->vertex_normals);
  free(m->vertex_tangents);
  free(m->vertex_binormals);
  free(m->vertex_colors);
  free(m->vertex_uvs);
  
  free(m->triangles);
  
  free(m);
  
}

render_model* to_render_model(model* m) {
  
  render_model* new_model = malloc(sizeof(render_model));
  new_model->name = malloc( strlen(m->name) + 1);
  strcpy(new_model->name, m->name);
  
  new_model->num_meshes = m->num_meshes;
  
  new_model->meshes = malloc(sizeof(render_mesh*) * new_model->num_meshes);
  
  int i;
  for( i=0; i < m->num_meshes; i++) {
    render_mesh* new_mesh = to_render_mesh(m->meshes[i]);
    new_model->meshes[i] = new_mesh;
  }
  
  return new_model;
};

render_model* render_model_from_render_mesh(render_mesh* m) {

  render_model* new_model = malloc(sizeof(render_model));
  new_model->name = m->name;
  new_model->num_meshes = 1;
  new_model->meshes = malloc(sizeof(render_mesh*) * new_model->num_meshes);
  new_model->meshes[1] = m;
  
  return new_model;

};

render_model* render_model_from_render_mesh(render_mesh* m);

void render_model_print(render_model* m) {
  
  printf("Model Name: %s\n", m->name);
  printf("Num meshes: %i\n", m->num_meshes);
  
  int i;
  for(i=0; i < m->num_meshes; i++) {
    render_mesh* me = m->meshes[i];
    render_mesh_print(me);
  }
  
};

void render_model_delete(render_model* m) {
  
  int i;
  for(i=0; i < m->num_meshes; i++) {
    render_mesh* me = m->meshes[i];
    render_mesh_delete(me);
  }
  
  free(m->name);
  free(m);
  
}

vector3 triangle_tangent(vertex vert1, vertex vert2, vertex vert3) {
  
  vector3 pos1 = vert1.position;
  vector3 pos2 = vert2.position;
  vector3 pos3 = vert3.position;
  
  vector2 uv1 = vert1.uvs;
  vector2 uv2 = vert2.uvs;
  vector2 uv3 = vert3.uvs;
  
  /* Get component Vectors */
  float x1 = pos2.x - pos1.x;
  float x2 = pos3.x - pos1.x;
  
  float y1 = pos2.y - pos1.y;
  float y2 = pos3.y - pos1.y;

  float z1 = pos2.z - pos1.z;
  float z2 = pos3.z - pos1.z;
  
  /* Generate uv space vectors */
  float s1 = uv2.x - uv1.x;
  float s2 = uv3.x - uv1.x;

  float t1 = uv2.y - uv1.y;
  float t2 = uv3.y - uv1.y;
  
  float r = 1.0f / ((s1 * t2) - (s2 * t1));
  
  vector3 tdir = v3(
          (s1 * x2 - s2 * x1) * r, 
          (s1 * y2 - s2 * y1) * r,
          (s1 * z2 - s2 * z1) * r
          );
  
  return v3_normalize(tdir);

};

vector3 triangle_binormal(vertex vert1, vertex vert2, vertex vert3) {
  
  vector3 pos1 = vert1.position;
  vector3 pos2 = vert2.position;
  vector3 pos3 = vert3.position;
  
  vector2 uv1 = vert1.uvs;
  vector2 uv2 = vert2.uvs;
  vector2 uv3 = vert3.uvs;
  
  /* Get component Vectors */
  float x1 = pos2.x - pos1.x;
  float x2 = pos3.x - pos1.x;
  
  float y1 = pos2.y - pos1.y;
  float y2 = pos3.y - pos1.y;

  float z1 = pos2.z - pos1.z;
  float z2 = pos3.z - pos1.z;
  
  /* Generate uv space vectors */
  float s1 = uv2.x - uv1.x;
  float s2 = uv3.x - uv1.x;

  float t1 = uv2.y - uv1.y;
  float t2 = uv3.y - uv1.y;
  
  float r = 1.0f / ((s1 * t2) - (s2 * t1));
  
  vector3 sdir = v3(
          (t2 * x1 - t1 * x2) * r, 
          (t2 * y1 - t1 * y2) * r,
          (t2 * z1 - t1 * z2) * r
          );
  
  return v3_normalize(sdir);

}

vector3 triangle_normal(vertex v1, vertex v2, vertex v3) {
  
  vector3 edge1 = v3_sub(v2.position, v1.position);
  vector3 edge2 = v3_sub(v3.position, v1.position);
  vector3 normal = v3_cross(edge1, edge2);
  
  return v3_normalize(normal);
  
}

float triangle_area(vertex v1, vertex v2, vertex v3) {
  
  vector3 ab = v3_sub(v1.position, v2.position);
  vector3 ac = v3_sub(v1.position, v3.position);
  
  float area = 0.5 * v3_length(v3_cross(ab, ac));
  
  //printf("Area: %f\n", area);
  
  return area;
  
}

vector3 triangle_random_position(vertex v1, vertex v2, vertex v3) {
  
  float r1 = (float)rand() / (float)RAND_MAX;
  float r2 = (float)rand() / (float)RAND_MAX;
  
  if(r1 + r2 >= 1) {
    r1 = 1 - r1;
    r2 = 1 - r2;
  }
  
  vector3 ab = v3_sub(v1.position, v2.position);
  vector3 ac = v3_sub(v1.position, v3.position);
  
  vector3 a = v1.position;
  a = v3_sub(a, v3_mul(ab , r1) );
  a = v3_sub(a, v3_mul(ac , r2) );
  
  return a;
  
}

/* CBM format - Corange Binary Model */

/* TODO: Change all int values in the model specification and loaders to longs */

/*
  
  -- Details --
  
  CBM
  [num_objects : int]
  [object list]
  
  -- Object --
  
  [name_len : int][name]
  [mat_name_len : int][mat_name]
  [num_verts : long]
    [verts : position, normal, tangent, binormal, uv, color]
  [num_triangles : long]
    [triangle_indicies : long]
    
*/

render_model* cbm_load_file(char* filename) {



}


void cbm_write_file(render_model* model) {



}
