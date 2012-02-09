#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vector.h"
#include "geometry.h"

plane plane_new(vector3 position, vector3 direction) {
  plane p;
  p.position = position;
  p.direction = direction;
  return p;
}

bool point_behind_plane(vector3 point, plane plane) {
  
  vector3 to_point = v3_sub(point, plane.position);
  float dist = v3_dot(to_point, plane.direction);
  
  if (dist < 0) {
    return true;
  } else {
    return false;
  }
  
}

float plane_signed_distance(plane plane, vector3 point) {
  vector3 to_point = v3_sub(point, plane.position);
  return v3_dot(to_point, plane.direction);
}

plane plane_transform(plane p, matrix_4x4 world) {
  
  p.position = m44_mul_v3(world, p.position);
  
  matrix_4x4 normworld = world;
  normworld.xw = 0; normworld.yw = 0; normworld.zw = 0;
  
  p.direction = m44_mul_v3(normworld, p.direction);
  
  return p;
}

box box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max) {

  box bb;
  bb.top    = plane_new( v3(0, y_max,0), v3(0, 1,0));
  bb.bottom = plane_new( v3(0, y_min,0), v3(0,-1,0));
  bb.left   = plane_new( v3( x_max,0,0), v3( 1,0,0));
  bb.right  = plane_new( v3( x_min,0,0), v3(-1,0,0));
  bb.front  = plane_new( v3(0,0, y_max), v3(0,0, 1));
  bb.back   = plane_new( v3(0,0, y_min), v3(0,0,-1));
  return bb;

}

box box_sphere(vector3 center, float radius) {
  
  box bb;
  bb.top    = plane_new(v3_add(center, v3(0, radius,0)), v3(0, 1,0));
  bb.bottom = plane_new(v3_add(center, v3(0,-radius,0)), v3(0,-1,0));
  bb.left   = plane_new(v3_add(center, v3( radius,0,0)), v3( 1,0,0));
  bb.right  = plane_new(v3_add(center, v3(-radius,0,0)), v3(-1,0,0));
  bb.front  = plane_new(v3_add(center, v3(0,0, radius)), v3(0,0, 1));
  bb.back   = plane_new(v3_add(center, v3(0,0,-radius)), v3(0,0,-1));
  return bb;
  
}

bool box_contains(box bb, vector3 point) {
  
  if ( !point_behind_plane(point, bb.top) ) { return false; }
  if ( !point_behind_plane(point, bb.bottom) ) { return false; }
  if ( !point_behind_plane(point, bb.left)) { return false; }
  if ( !point_behind_plane(point, bb.right)) { return false; }
  if ( !point_behind_plane(point, bb.front)) { return false; }
  if ( !point_behind_plane(point, bb.back)) { return false; }
  
  return true;
}

box box_merge(box b1, box b2) {
  
  float b1_x_max = b1.left.position.x;
  float b1_x_min = b1.right.position.x;
  float b1_y_max = b1.top.position.y;
  float b1_y_min = b1.bottom.position.y;
  float b1_z_max = b1.front.position.z;
  float b1_z_min = b1.back.position.z;
  
  float b2_x_max = b2.left.position.x;
  float b2_x_min = b2.right.position.x;
  float b2_y_max = b2.top.position.y;
  float b2_y_min = b2.bottom.position.y;
  float b2_z_max = b2.front.position.z;
  float b2_z_min = b2.back.position.z;
  
  float x_min = min(b1_x_min, b2_x_min);
  float x_max = max(b1_x_max, b2_x_max);
  float y_min = min(b1_y_min, b2_y_min);
  float y_max = max(b1_y_max, b2_y_max);
  float z_min = min(b1_z_min, b2_z_min);
  float z_max = max(b1_z_max, b2_z_max);
  
  return box_new(x_min, x_max, y_min, y_max, z_min, z_max);
}

box box_transform(box bb, matrix_4x4 world_matrix) {
  
  bb.top = plane_transform(bb.top, world_matrix);
  bb.bottom = plane_transform(bb.bottom, world_matrix);
  bb.left = plane_transform(bb.left, world_matrix);
  bb.right = plane_transform(bb.right, world_matrix);
  bb.front = plane_transform(bb.front, world_matrix);
  bb.back = plane_transform(bb.back, world_matrix);
  
  return bb;
  
}

sphere sphere_new(vector3 center, float radius) {
  sphere bs;
  bs.center = center;
  bs.radius = radius;
  bs.radius_sqrd = radius * radius;
  
  return bs;
}

sphere sphere_of_box(box bb) {
  
  float x_max = bb.left.position.x;
  float x_min = bb.right.position.x;
  float y_max = bb.top.position.y;
  float y_min = bb.bottom.position.y;
  float z_max = bb.front.position.z;
  float z_min = bb.back.position.z;
  
  vector3 center;
  center.x = (x_min + x_max) / 2;
  center.y = (y_min + y_max) / 2;
  center.z = (z_min + z_max) / 2;
  
  float radius = 0;
  radius = max(radius, v3_dist(center, v3(x_min, y_min, z_min)));
  radius = max(radius, v3_dist(center, v3(x_max, y_min, z_min)));
  radius = max(radius, v3_dist(center, v3(x_min, y_max, z_min)));
  radius = max(radius, v3_dist(center, v3(x_min, y_min, z_max)));
  radius = max(radius, v3_dist(center, v3(x_min, y_max, z_max)));
  radius = max(radius, v3_dist(center, v3(x_max, y_max, z_min)));
  radius = max(radius, v3_dist(center, v3(x_max, y_min, z_max)));
  radius = max(radius, v3_dist(center, v3(x_max, y_max, z_max)));
  
  sphere bs;
  bs.center = center;
  bs.radius = radius;
  bs.radius_sqrd = radius * radius;
  
  return bs;
}

sphere sphere_merge(sphere bs1, sphere bs2) {
  
  vector3 center = v3_div(v3_add(bs1.center, bs2.center), 2);
  
  vector3 dir = v3_normalize(v3_sub(bs2.center, bs1.center));
  vector3 edge = v3_add(v3_mul(dir, bs2.radius), bs2.center);
  
  float dist = v3_dist(edge, center);
  
  sphere bs;
  bs.center = center;
  bs.radius = dist;
  bs.radius_sqrd = dist * dist;
  
  return bs;
}

bool sphere_contains_point(sphere s1, vector3 point) {
  float dist_sqrt = v3_dist_sqrd(s1.center, point);
  return dist_sqrt <= s1.radius_sqrd;
}

bool sphere_contains_sphere(sphere s1, sphere s2) {
  float dist_sqrt = v3_dist_sqrd(s1.center, s2.center);
  return dist_sqrt <= s1.radius_sqrd + s2.radius_sqrd;
}

sphere sphere_transform(sphere bs, matrix_4x4 world) {
  
  vector3 center = m44_mul_v3(world, bs.center);
  float radius = bs.radius * max(max(world.xx, world.yy), world.zz);
  
  sphere b;
  b.center = center;
  b.radius = radius;
  b.radius_sqrd = radius * radius;
  
  return b;
}

vertex vertex_new() {
  vertex v;
  memset(&v, 0, sizeof(vertex));
  return v;
}

bool vertex_equal(vertex v1, vertex v2) {
  
  if(!v3_equ(v1.position, v2.position)) { return false; }
  if(!v3_equ(v1.normal, v2.normal)) { return false; }
  if(!v2_equ(v1.uvs, v2.uvs)) { return false; }
  
  return true;  
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

void mesh_print(mesh* m) {
  
  printf("Mesh Name: %s\n", m->name);
  printf("Material Name: %s\n", m->material);
  printf("Num Verts: %i\n", m->num_verts);
  printf("Num Tris: %i\n", m->num_triangles);
  for(int i=0; i < m->num_verts; i++) {
    vertex_print(m->verticies[i]); printf("\n");
  }
  printf("Triangle Indicies:");
  for(int i=0; i < m->num_triangles_3; i++) {
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
  
  /* Clear all tangents to 0,0,0 */
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_zero();
    m->verticies[i].binormal = v3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  
  int i = 0;
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
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = v3_normalize( m->verticies[i].binormal );
  }
  
}

void mesh_generate_normals(mesh* m) {
  
  /* Clear all normals to 0,0,0 */
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].normal = v3_zero();
  }
  
  /* Loop over faces, calculate normals and append to verticies of that face */
  
  int i = 0;
  while( i < m->num_triangles_3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vector3 face_normal = triangle_normal(v1, v2, v3);
    
    v1.normal = v3_add(face_normal, v1.normal);
    v2.normal = v3_add(face_normal, v2.normal);
    v3.normal = v3_add(face_normal, v3.normal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  /* normalize all normals */
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].normal = v3_normalize( m->verticies[i].normal );
  }
  
}

void mesh_generate_orthagonal_tangents(mesh* m) {
  
  /* Clear all tangents to 0,0,0 */
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_zero();
    m->verticies[i].binormal = v3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  
  int i = 0;
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
  
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = v3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = v3_normalize( m->verticies[i].binormal );
  }
  
}

void mesh_generate_texcoords_cylinder(mesh* m) {
  
	vector2 unwrap_vector = v2(1, 0);
	
	float max_height = -99999999;
	float min_height = 99999999;
	
	for(int i = 0; i < m->num_verts; i++) {
		float v = m->verticies[i].position.y;
		max_height = max(max_height, v);
		min_height = min(min_height, v);
		
		vector2 proj_position = v2(m->verticies[i].position.x, m->verticies[i].position.z);
		vector2 from_center = v2_normalize(proj_position);
		float u = (v2_dot(from_center, unwrap_vector) + 1) / 8;
		
		m->verticies[i].uvs = v2(u, v);
	}
	
	float scale = (max_height - min_height);
	
	for(int i = 0; i < m->num_verts; i++) {
		m->verticies[i].uvs = v2(m->verticies[i].uvs.x, m->verticies[i].uvs.y / scale);
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
  for(int i = m1->num_meshes; i < total_num_meshes;i++) {
    m1->meshes[i] = m2->meshes[i - m1->num_meshes];
  }
  
  m1->num_meshes = total_num_meshes;

  free(m2->name);
  free(m2);
}

void model_print(model* m) {
  for(int i=0; i<m->num_meshes; i++) {
    mesh_print( m->meshes[i] );
  }
}

void model_delete(model* m) {
  
  for(int i=0; i<m->num_meshes; i++) {
    mesh_delete( m->meshes[i] );
  }
  
  free(m->name);
  free(m);
  
}

void model_generate_normals(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_normals( m->meshes[i] );
  }
  
}

void model_generate_tangents(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_tangents( m->meshes[i] );
  }
  
}

void model_generate_orthagonal_tangents(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_orthagonal_tangents( m->meshes[i] );
  }

}

void model_generate_texcoords_cylinder(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_texcoords_cylinder( m->meshes[i] );
  }

}

float model_surface_area(model* m) {
  float total = 0.0f;
  
  for(int i = 0; i < m->num_meshes; i++) {
    total += mesh_surface_area( m->meshes[i] );
  }
  
  return total;
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

vertex triangle_random_position_interpolation(vertex v1, vertex v2, vertex v3) {

  float r1 = (float)rand() / (float)RAND_MAX;
  float r2 = (float)rand() / (float)RAND_MAX;
  
  if(r1 + r2 >= 1) {
    r1 = 1 - r1;
    r2 = 1 - r2;
  }
  
  vertex v;
  
  vector3 v_pos, v_norm, v_tang, v_binorm;
  vector4 v_col;
  vector2 v_uv;
  
  v_pos = v1.position;
  v_pos = v3_sub(v_pos, v3_mul(v3_sub(v1.position, v2.position) , r1) );
  v_pos = v3_sub(v_pos, v3_mul(v3_sub(v1.position, v3.position) , r2) );
  
  v_norm = v1.normal;
  v_norm = v3_sub(v_norm, v3_mul(v3_sub(v1.normal, v2.normal) , r1) );
  v_norm = v3_sub(v_norm, v3_mul(v3_sub(v1.normal, v3.normal) , r2) );
  
  v_tang = v1.tangent;
  v_tang = v3_sub(v_tang, v3_mul(v3_sub(v1.tangent, v2.tangent) , r1) );
  v_tang = v3_sub(v_tang, v3_mul(v3_sub(v1.tangent, v3.tangent) , r2) );
  
  v_binorm = v1.binormal;
  v_binorm = v3_sub(v_binorm, v3_mul(v3_sub(v1.binormal, v2.binormal) , r1) );
  v_binorm = v3_sub(v_binorm, v3_mul(v3_sub(v1.binormal, v3.binormal) , r2) );
  
  v_col = v1.color;
  v_col = v4_sub(v_col, v4_mul(v4_sub(v1.color, v2.color) , r1) );
  v_col = v4_sub(v_col, v4_mul(v4_sub(v1.color, v3.color)  , r2) );
  
  v_uv = v1.uvs;
  v_uv = v2_sub(v_uv, v2_mul(v2_sub(v1.uvs, v2.uvs) , r1) );
  v_uv = v2_sub(v_uv, v2_mul(v2_sub(v1.uvs, v3.uvs)  , r2) );
  
  v.position = v_pos;
  v.normal = v_norm;
  v.tangent = v_tang;
  v.binormal = v_binorm;
  v.color = v_col;
  v.uvs = v_uv;
  
  return v;
}


float triangle_difference_u(vertex v1, vertex v2, vertex v3) {
  
  float max = v1.uvs.x;
  max = v2.uvs.x > max ? v2.uvs.x : max;
  max = v3.uvs.x > max ? v3.uvs.x : max;
  
  float min = v1.uvs.x;
  min = v2.uvs.x < min ? v2.uvs.x : min;
  min = v3.uvs.x < min ? v3.uvs.x : min;
  
  return max - min;
  
}

float triangle_difference_v(vertex v1, vertex v2, vertex v3) {

  float max = v1.uvs.y;
  max = v2.uvs.x > max ? v2.uvs.y : max;
  max = v3.uvs.x > max ? v3.uvs.y : max;
  
  float min = v1.uvs.y;
  min = v2.uvs.y < min ? v2.uvs.y : min;
  min = v3.uvs.y < min ? v3.uvs.y : min;
  
  return max - min;

}
