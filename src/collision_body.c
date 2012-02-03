#include <stdlib.h>
#include <math.h>

#include "SDL/SDL_RWops.h"

#include "error.h"
#include "matrix.h"
#include "int_list.h"
#include "vertex_list.h"
#include "vertex_hashtable.h"

#include "collision_body.h"

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

plane plane_transform(plane p, matrix_4x4 world) {
  
  p.position = m44_mul_v3(world, p.position);
  
  matrix_4x4 normworld = world;
  normworld.xw = 0; normworld.yw = 0; normworld.zw = 0;
  
  p.direction = m44_mul_v3(normworld, p.direction);
  
  return p;
}

bounding_box bounding_box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max) {

  bounding_box bb;
  bb.top    = plane_new( v3(0, y_max,0), v3(0, 1,0));
  bb.bottom = plane_new( v3(0, y_min,0), v3(0,-1,0));
  bb.left   = plane_new( v3( x_max,0,0), v3( 1,0,0));
  bb.right  = plane_new( v3( x_min,0,0), v3(-1,0,0));
  bb.front  = plane_new( v3(0,0, y_max), v3(0,0, 1));
  bb.back   = plane_new( v3(0,0, y_min), v3(0,0,-1));
  return bb;

}

bounding_box bounding_box_sphere(vector3 center, float radius) {
  
  bounding_box bb;
  bb.top    = plane_new(v3_add(center, v3(0, radius,0)), v3(0, 1,0));
  bb.bottom = plane_new(v3_add(center, v3(0,-radius,0)), v3(0,-1,0));
  bb.left   = plane_new(v3_add(center, v3( radius,0,0)), v3( 1,0,0));
  bb.right  = plane_new(v3_add(center, v3(-radius,0,0)), v3(-1,0,0));
  bb.front  = plane_new(v3_add(center, v3(0,0, radius)), v3(0,0, 1));
  bb.back   = plane_new(v3_add(center, v3(0,0,-radius)), v3(0,0,-1));
  return bb;
  
}

bool bounding_box_contains(bounding_box bb, vector3 point) {
  
  if ( !point_behind_plane(point, bb.top) ) { return false; }
  if ( !point_behind_plane(point, bb.bottom) ) { return false; }
  if ( !point_behind_plane(point, bb.left)) { return false; }
  if ( !point_behind_plane(point, bb.right)) { return false; }
  if ( !point_behind_plane(point, bb.front)) { return false; }
  if ( !point_behind_plane(point, bb.back)) { return false; }
  
  return true;
}

bounding_box bounding_box_merge(bounding_box b1, bounding_box b2) {
  
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
  
  return bounding_box_new(x_min, x_max, y_min, y_max, z_min, z_max);
}

bounding_box bounding_box_transform(bounding_box bb, matrix_4x4 world_matrix) {
  
  bb.top = plane_transform(bb.top, world_matrix);
  bb.bottom = plane_transform(bb.bottom, world_matrix);
  bb.left = plane_transform(bb.left, world_matrix);
  bb.right = plane_transform(bb.right, world_matrix);
  bb.front = plane_transform(bb.front, world_matrix);
  bb.back = plane_transform(bb.back, world_matrix);
  
  return bb;
  
}

bounding_sphere bounding_sphere_new(vector3 center, float radius) {
  bounding_sphere bs;
  bs.center = center;
  bs.radius = radius;
  bs.radius_sqrd = radius * radius;
  
  return bs;
}

bounding_sphere bounding_sphere_of_box(bounding_box bb) {
  
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
  
  bounding_sphere bs;
  bs.center = center;
  bs.radius = radius;
  bs.radius_sqrd = radius * radius;
  
  return bs;
}

bounding_sphere bounding_sphere_merge(bounding_sphere bs1, bounding_sphere bs2) {
  
  vector3 center = v3_div(v3_add(bs1.center, bs2.center), 2);
  
  vector3 dir = v3_normalize(v3_sub(bs2.center, bs1.center));
  vector3 edge = v3_add(v3_mul(dir, bs2.radius), bs2.center);
  
  float dist = v3_dist(edge, center);
  
  bounding_sphere bs;
  bs.center = center;
  bs.radius = dist;
  bs.radius_sqrd = dist * dist;
  
  return bs;
}

bool bounding_sphere_contains(bounding_sphere bs1, vector3 point) {
  float dist_sqrt = v3_dist_sqrd(bs1.center, point);
  return dist_sqrt <= bs1.radius_sqrd;
}

bounding_sphere bounding_sphere_transform(bounding_sphere bs, matrix_4x4 world) {
  
  vector3 center = m44_mul_v3(world, bs.center);
  float radius = bs.radius * max(max(world.xx, world.yy), world.zz);
  
  bounding_sphere b;
  b.center = center;
  b.radius = radius;
  b.radius_sqrd = radius * radius;
  
  return b;
}

void collision_mesh_delete(collision_mesh* cm) {
  
  if (cm->is_leaf) {
    free(cm->verticies);
    free(cm->triangle_normals);
    free(cm);
  } else {
    collision_mesh_delete(cm->front);
    collision_mesh_delete(cm->back);
    free(cm);
  }
  
}

static vector3 collision_mesh_vertex_average(collision_mesh* cm) {

  vector3 midpoint = v3_zero();
  for(int i = 0; i < cm->num_verticies; i++) {
    midpoint = v3_add(midpoint, cm->verticies[i]);
  }
  midpoint = v3_div(midpoint, cm->num_verticies);
  
  return midpoint;

}

static plane collision_mesh_division(collision_mesh* cm) {
  
  bounding_box bb = collision_mesh_bounding_box(cm);
  
  plane p;
  p.position = collision_mesh_vertex_average(cm);
  
  float x_diff = bb.left.position.x - bb.right.position.x;
  float y_diff = bb.top.position.y - bb.bottom.position.y;
  float z_diff = bb.front.position.z - bb.back.position.z;
  
  if ((x_diff >= y_diff) && (x_diff >= z_diff)) {
    p.direction = v3(1,0,0);
  } else if ((y_diff >= x_diff) && (y_diff >= z_diff)) {
    p.direction = v3(0,1,0); 
  } else if ((z_diff >= x_diff) && (z_diff >= y_diff)) {
    p.direction = v3(0,0,1);
  }
  
  return p;
}


void collision_mesh_subdivide(collision_mesh* cm, int iterations) {
  
  if (iterations == 0) { return; }
  
  if (!cm->is_leaf) {
    error("Attempt to subdivide non-leaf bsp tree!");
  }
  
  cm->division = collision_mesh_division(cm);
  
  int num_front = 0;
  int num_back = 0;
  
  for(int i = 0; i < cm->num_verticies / 3; i++) {
  
    vector3 p1 = cm->verticies[i*3+0];
    vector3 p2 = cm->verticies[i*3+1];
    vector3 p3 = cm->verticies[i*3+2];
    
    if (point_behind_plane(p1, cm->division) &&
        point_behind_plane(p2, cm->division) &&
        point_behind_plane(p3, cm->division)) {
      num_back += 3;
    } else if ((!point_behind_plane(p1, cm->division)) &&
               (!point_behind_plane(p2, cm->division)) &&
               (!point_behind_plane(p3, cm->division))) {
      num_front += 3;
    } else {
      num_back += 3;
      num_front += 3;
    }
  }
  
  cm->front = malloc(sizeof(collision_mesh));
  cm->front->is_leaf = true;
  cm->front->front = NULL;
  cm->front->back = NULL;
  cm->front->division.position = v3_zero();
  cm->front->division.direction = v3_zero();
  cm->front->verticies = malloc(sizeof(vector3) * num_front);
  cm->front->num_verticies = num_front;
  cm->front->triangle_normals = malloc(sizeof(vector3) * (num_front / 3));
  
  cm->back = malloc(sizeof(collision_mesh));
  cm->back->is_leaf = true;
  cm->back->front = NULL;
  cm->back->back = NULL;
  cm->back->division.position = v3_zero();
  cm->back->division.direction = v3_zero();
  cm->back->verticies = malloc(sizeof(vector3) * num_back);
  cm->back->num_verticies = num_back;
  cm->back->triangle_normals = malloc(sizeof(vector3) * (num_back / 3));
  
  int front_i = 0;
  int back_i = 0;
  
  for(int i = 0; i < cm->num_verticies / 3; i++) {
  
    vector3 p1 = cm->verticies[i*3+0];
    vector3 p2 = cm->verticies[i*3+1];
    vector3 p3 = cm->verticies[i*3+2];
    
    vector3 norm = cm->triangle_normals[i];
    
    if (point_behind_plane(p1, cm->division) &&
        point_behind_plane(p2, cm->division) &&
        point_behind_plane(p3, cm->division)) {
      
      cm->back->triangle_normals[back_i / 3] = norm;
      cm->back->verticies[back_i] = p1; back_i++;
      cm->back->verticies[back_i] = p2; back_i++;
      cm->back->verticies[back_i] = p3; back_i++;
      
    } else if ((!point_behind_plane(p1, cm->division)) &&
               (!point_behind_plane(p2, cm->division)) &&
               (!point_behind_plane(p3, cm->division))) {
               
      cm->front->triangle_normals[front_i / 3] = norm;
      cm->front->verticies[front_i] = p1; front_i++;
      cm->front->verticies[front_i] = p2; front_i++;
      cm->front->verticies[front_i] = p3; front_i++;
      
    } else {
      
      cm->back->triangle_normals[back_i / 3] = norm;
      cm->back->verticies[back_i] = p1; back_i++;
      cm->back->verticies[back_i] = p2; back_i++;
      cm->back->verticies[back_i] = p3; back_i++;
      cm->front->triangle_normals[front_i / 3] = norm;
      cm->front->verticies[front_i] = p1; front_i++;
      cm->front->verticies[front_i] = p2; front_i++;
      cm->front->verticies[front_i] = p3; front_i++;
      
    }
    
  }
  
  free(cm->verticies);
  free(cm->triangle_normals);
  
  cm->is_leaf = false;
  cm->verticies = NULL;
  cm->triangle_normals = NULL;
  cm->num_verticies = 0;
  
  collision_mesh_subdivide(cm->front, iterations-1);
  collision_mesh_subdivide(cm->back, iterations-1);
  
}

bounding_sphere collision_mesh_bounding_sphere(collision_mesh* cm) {
  
  if (cm->is_leaf) {
    
    bounding_box bb = collision_mesh_bounding_box(cm);
    vector3 center;
    center.x = (bb.left.position.x + bb.right.position.x) / 2;
    center.y = (bb.top.position.y + bb.bottom.position.y) / 2;
    center.z = (bb.front.position.z + bb.back.position.z) / 2;
    
    float dist = 0;
    for(int i = 0; i < cm->num_verticies; i++) {
      dist = max(dist, v3_dist(cm->verticies[i], center));
    }
    
    bounding_sphere bs;
    bs.center = center;
    bs.radius = dist;
    bs.radius_sqrd = dist * dist;
    
    return bs;
    
  } else {
    bounding_sphere front_bs = collision_mesh_bounding_sphere(cm->front);
    bounding_sphere back_bs = collision_mesh_bounding_sphere(cm->back);
    return bounding_sphere_merge(front_bs, back_bs);
  }
  
}

bounding_box collision_mesh_bounding_box(collision_mesh* cm) {

  if (cm->is_leaf) {
    
    float x_min = 10000; float x_max = -10000;
    float y_min = 10000; float y_max = -10000;
    float z_min = 10000; float z_max = -10000;
    
    for(int i = 0; i < cm->num_verticies; i++) {
      x_min = min(x_min, cm->verticies[i].x);
      x_max = max(x_max, cm->verticies[i].x);
      y_min = min(y_min, cm->verticies[i].y);
      y_max = max(y_max, cm->verticies[i].y);
      z_min = min(z_min, cm->verticies[i].z);
      z_max = max(z_max, cm->verticies[i].z);
    }
    
    return bounding_box_new(x_min, x_max, y_min, y_max, z_min, z_max);
    
  } else {
    bounding_box front_box = collision_mesh_bounding_box(cm->front);
    bounding_box back_box = collision_mesh_bounding_box(cm->back);
    return bounding_box_merge(front_box, back_box);
  }

}

collision_body* collision_body_new_sphere(bounding_sphere bs) {
  
  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_sphere;
  cb->collision_sphere = bs;
  cb->collision_box = bounding_box_sphere(bs.center, bs.radius);
  cb->collision_mesh = NULL;
  
  return cb;
  
}

collision_body* collision_body_new_box(bounding_box bb) {

  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_box;
  cb->collision_sphere = bounding_sphere_of_box(bb);
  cb->collision_box = bb;
  cb->collision_mesh = NULL;

  return cb;
}

collision_body* collision_body_new_mesh(collision_mesh* cm) {

  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_mesh;
  cb->collision_sphere = collision_mesh_bounding_sphere(cm);
  cb->collision_box = collision_mesh_bounding_box(cm);
  cb->collision_mesh = cm;

  return cb;
}

void collision_body_delete(collision_body* cb) {
  
  if (cb->collision_mesh != NULL) {
    collision_mesh_delete(cb->collision_mesh);
  }
  free(cb);
  
}

collision_mesh* col_load_file(char* filename) {
  
  collision_mesh* mesh = malloc(sizeof(collision_mesh));
  mesh->division.position = v3_zero();
  mesh->division.direction = v3_zero();
  mesh->front = NULL;
  mesh->back = NULL;
  mesh->is_leaf = true;
  
  vertex_list* vert_data = vertex_list_new_blocksize(1024);
  vertex_list* vert_list = vertex_list_new_blocksize(1024);
  int_list* tri_list = int_list_new_blocksize(1024);
  vertex_hashtable* vert_hashes = vertex_hashtable_new(4096);
  
  int num_pos, num_norm, num_tex;
  num_pos = num_norm = num_tex = 0;
  int vert_index = 0;
  
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
      /* Do nothing */
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
      /* Do nothing */
    }
    
    else if (sscanf(line, "usemtl %512s", material) == 1) {
      /* Do nothing */
    }
    
    else if (sscanf(line, "s %i", &smoothing_group) == 1) {
      /* Smoothing group, do nothing */
    }
    
    else if (sscanf(line, "f %i/%i/%i %i/%i/%i %i/%i/%i", &pi1, &ti1, &ni1, &pi2, &ti2, &ni2, &pi3, &ti3, &ni3) == 9) {
      
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

  mesh->num_verticies = tri_list->num_items;
  
  mesh->verticies = malloc(sizeof(vector3) * mesh->num_verticies);
  mesh->triangle_normals = malloc(sizeof(vector3) * mesh->num_verticies / 3);
  
  for(int i = 0; i < mesh->num_verticies / 3; i++) {
    int i1 = int_list_get(tri_list, i*3+0);
    int i2 = int_list_get(tri_list, i*3+1);
    int i3 = int_list_get(tri_list, i*3+2);
    
    vertex v1 = vertex_list_get(vert_list, i1);
    vertex v2 = vertex_list_get(vert_list, i2);
    vertex v3 = vertex_list_get(vert_list, i3);
    
    mesh->verticies[i*3+0] = v1.position;
    mesh->verticies[i*3+1] = v2.position;
    mesh->verticies[i*3+2] = v3.position;
    
    vector3 avg_norm = v3_add(v1.normal, v3_add(v2.normal, v3.normal)); 
    avg_norm = v3_div(avg_norm, 3);
    
    vector3 norm1 = triangle_normal(v1, v2, v3);
    vector3 norm2 = v3_neg(triangle_normal(v1, v2, v3));
    
    float norm1_angle = 1 - v3_dot(avg_norm, norm1);
    float norm2_angle = 1 - v3_dot(avg_norm, norm2);
    
    if (norm1_angle < norm2_angle) {
      mesh->triangle_normals[i] = norm1;
    } else {
      mesh->triangle_normals[i] = norm2;
    }
    
  }
  
  vertex_hashtable_delete(vert_hashes);
  vertex_list_delete(vert_data);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  
  collision_mesh_subdivide(mesh, 5);
  
  return mesh;
}

/*
  First, add object radius to target radius.
  We now consider object as a ray.
  
  Shift target to origin (also shifting ray obviously). This simplifies things.
  Point intersects with target when.
  
  p dot p = r*r
  p = O + t * V
  (O + t*V) dot (O + t*V) = r*r
  
  expands to quadratic in the form A^2 + B + C = 0 where
  
  A = (V dot V)
  B = 2(V dot O)
  C = (O dot O) - r*r
  
  Solutions given by: ( -B +-sqrt(B*B - 4*A*C) ) / 2A
  
  And if the descriminate (B*B - 4*A*C) < 0 we know as an early out that the ray has missed the sphere.
*/

void sphere_collide_sphere(collision* out, bounding_sphere object, vector3 object_velocity, bounding_sphere target, float timestep) {
  
  float r = object.radius + target.radius;
  
  vector3 V = object_velocity;
  vector3 O = v3_sub(object.center, target.center);
  
  float A = v3_dot(V, V);
  float B = 2 * v3_dot(V, O);
  float C = v3_dot(O, O) - (r*r);
  
  float descrim = B*B - 4*A*C;
  if (descrim < 0) {
    out->collided = false;
    return;
  }
  
  float dist_sqrd = sqrtf(descrim);
  
  /* This intermediate is used to reduce numerical instability */
  float q;
  if (B < 0) q = (-B - dist_sqrd)/2.0;
  else q = (-B + dist_sqrd)/2.0;
  
  float t0 = q / A;
  float t1 = C / q;
  
  /* If outside of time range we don't care about them this frame */
  if (((t0 < 0) || (t0 > timestep)) &&
      ((t1 < 0) || (t1 > timestep))) {
    out->collided = false;
    return;
  }
  
  float time = 0;
  if ((t0 >= 0) && (t0 <= timestep) &&
      (t1 >= 0) && (t1 <= timestep)) {
    time = min(t0, t1);
  } else if ((t0 >= 0) && (t0 <= timestep)) {
    time = t0;
  } else if ((t1 >= 0) && (t1 <= timestep))  {
    time = t1;
  }
  
  out->collided = true;
  out->time = time;
  out->object_position = v3_add(object.center, v3_mul(object_velocity, time - 0.001));
  out->surface_normal = v3_normalize(v3_sub(out->object_position, target.center));
  out->surface_position = v3_add(target.center, v3_mul(out->surface_normal, target.radius));
  
  return;
  
}

/*
  Time for collision between plane and sphere
  
  Translate sphere to origin, and obviously also translate plane.
  
  SignedDistance(p) = N dot p + C
  
  SignedDistance(P0 + t * V) = r
  N dot (P0 + t * V) + C = r
  N dot P0 + t * (N dot V) + C = r
  t * (N dot V) + SignedDistance(P0) = r
  
  t = (r - SignedDistance(P0)) / N dot V
  
  We can get two versions of t - t0 for when r is positive and t1 for when r is negative.
  
  If both t are outside range 0..time_step we know there is no collision.
  
  Otherwise we have a collision sometime between t0..t1
  
*/

void sphere_collide_mesh(collision* out, bounding_sphere object, vector3 object_velocity, collision_mesh* target, matrix_4x4 target_world, float timestep) {

  
  
  
}