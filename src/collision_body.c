#include <stdlib.h>
#include <math.h>

#include "SDL/SDL_rwops.h"

#include "error.h"
#include "matrix.h"
#include "int_list.h"
#include "vertex_list.h"
#include "vertex_hashtable.h"

#include "collision_body.h"

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
  p.direction = m44_mul_v3(world, p.direction);
  return p;
}

bounding_box bounding_box_sphere(vector3 center, float radius) {
  
  bounding_box bb;
  bb.x_min = center.x - radius;
  bb.x_max = center.x + radius;
  bb.y_min = center.y - radius;
  bb.y_max = center.y + radius;
  bb.z_min = center.z - radius;
  bb.z_max = center.z + radius;
  
  return bb;
  
}

bool bounding_box_contains(bounding_box bb, vector3 point) {
  
  if (point.x > bb.x_max) { return false; }
  if (point.x < bb.x_min) { return false; }
  if (point.y > bb.y_max) { return false; }
  if (point.y < bb.y_min) { return false; }
  if (point.z > bb.z_max) { return false; }
  if (point.z < bb.z_min) { return false; }
  
  return true;
}

bounding_box bounding_box_merge(bounding_box b1, bounding_box b2) {
  
  bounding_box bb;
  bb.x_min = min(b1.x_min, b2.x_min);
  bb.x_max = max(b1.x_max, b2.x_max);
  bb.y_min = min(b1.y_min, b2.y_min);
  bb.y_max = max(b1.y_max, b2.y_max);
  bb.z_min = min(b1.z_min, b2.z_min);
  bb.z_max = max(b1.z_max, b2.z_max);
  
  return bb;
}

bounding_box bounding_box_transform(bounding_box bb, matrix_4x4 world_matrix) {
  
  warning("bounding_box_transform not implemented yet!");
  
  return bb;
  
}

bounding_sphere bounding_sphere_of_box(bounding_box bb) {
  
  vector3 center;
  center.x = (bb.x_min + bb.x_max) / 2;
  center.y = (bb.y_min + bb.y_max) / 2;
  center.z = (bb.z_min + bb.z_max) / 2;
  
  float radius = 0;
  radius = max(radius, v3_dist(center, v3(bb.x_min, bb.y_min, bb.z_min)));
  radius = max(radius, v3_dist(center, v3(bb.x_max, bb.y_min, bb.z_min)));
  radius = max(radius, v3_dist(center, v3(bb.x_min, bb.y_max, bb.z_min)));
  radius = max(radius, v3_dist(center, v3(bb.x_min, bb.y_min, bb.z_max)));
  radius = max(radius, v3_dist(center, v3(bb.x_min, bb.y_max, bb.z_max)));
  radius = max(radius, v3_dist(center, v3(bb.x_max, bb.y_max, bb.z_min)));
  radius = max(radius, v3_dist(center, v3(bb.x_max, bb.y_min, bb.z_max)));
  radius = max(radius, v3_dist(center, v3(bb.x_max, bb.y_max, bb.z_max)));
  
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

void bsp_mesh_delete(bsp_mesh* bm) {
  
  if (bm->is_leaf) {
    free(bm->verticies);
    free(bm->triangle_normals);
    free(bm);
  } else {
    bsp_mesh_delete(bm->front);
    bsp_mesh_delete(bm->back);
    free(bm);
  }
  
}

static vector3 bsp_mesh_vertex_average(bsp_mesh* bm) {

  vector3 midpoint = v3_zero();
  for(int i = 0; i < bm->num_verticies; i++) {
    midpoint = v3_add(midpoint, bm->verticies[i]);
  }
  midpoint = v3_div(midpoint, bm->num_verticies);
  
  return midpoint;

}

static plane bsp_mesh_division(bsp_mesh* bm) {
  
  bounding_box bb = bsp_mesh_bounding_box(bm);
  
  plane p;
  p.position = bsp_mesh_vertex_average(bm);
  
  float x_diff = bb.x_max - bb.x_min;
  float y_diff = bb.y_max - bb.y_min;
  float z_diff = bb.z_max - bb.z_min;
  
  if ((x_diff >= y_diff) && (x_diff >= z_diff)) {
    p.direction = v3(1,0,0);
  } else if ((y_diff >= x_diff) && (y_diff >= z_diff)) {
    p.direction = v3(0,1,0); 
  } else if ((z_diff >= x_diff) && (z_diff >= y_diff)) {
    p.direction = v3(0,0,1);
  }
  
  return p;
}


void bsp_mesh_subdivide(bsp_mesh* bm, int iterations) {
  
  if (iterations == 0) { return; }
  
  if (!bm->is_leaf) {
    error("Attempt to subdivide non-leaf bsp tree!");
  }
  
  bm->division = bsp_mesh_division(bm);
  
  int num_front = 0;
  int num_back = 0;
  
  for(int i = 0; i < bm->num_verticies / 3; i++) {
  
    vector3 p1 = bm->verticies[i*3+0];
    vector3 p2 = bm->verticies[i*3+1];
    vector3 p3 = bm->verticies[i*3+2];
    
    if (point_behind_plane(p1, bm->division) &&
        point_behind_plane(p2, bm->division) &&
        point_behind_plane(p3, bm->division)) {
      num_back += 3;
    } else if ((!point_behind_plane(p1, bm->division)) &&
               (!point_behind_plane(p2, bm->division)) &&
               (!point_behind_plane(p3, bm->division))) {
      num_front += 3;
    } else {
      num_back += 3;
      num_front += 3;
    }
  }
  
  debug("-- Iter %i --", iterations);
  debug("Front: %i", num_front/3);
  debug("Back: %i", num_back/3);
  debug("-------------")
  
  bm->front = malloc(sizeof(bsp_mesh));
  bm->front->is_leaf = true;
  bm->front->front = NULL;
  bm->front->back = NULL;
  bm->front->division.position = v3_zero();
  bm->front->division.direction = v3_zero();
  bm->front->verticies = malloc(sizeof(vector3) * num_front);
  bm->front->num_verticies = num_front;
  bm->front->triangle_normals = malloc(sizeof(vector3) * (num_front / 3));
  
  bm->back = malloc(sizeof(bsp_mesh));
  bm->back->is_leaf = true;
  bm->back->front = NULL;
  bm->back->back = NULL;
  bm->back->division.position = v3_zero();
  bm->back->division.direction = v3_zero();
  bm->back->verticies = malloc(sizeof(vector3) * num_back);
  bm->back->num_verticies = num_back;
  bm->back->triangle_normals = malloc(sizeof(vector3) * (num_back / 3));
  
  int front_i = 0;
  int back_i = 0;
  
  for(int i = 0; i < bm->num_verticies / 3; i++) {
  
    vector3 p1 = bm->verticies[i*3+0];
    vector3 p2 = bm->verticies[i*3+1];
    vector3 p3 = bm->verticies[i*3+2];
    
    vector3 norm = bm->triangle_normals[i];
    
    if (point_behind_plane(p1, bm->division) &&
        point_behind_plane(p2, bm->division) &&
        point_behind_plane(p3, bm->division)) {
      
      bm->back->triangle_normals[back_i / 3] = norm;
      bm->back->verticies[back_i] = p1; back_i++;
      bm->back->verticies[back_i] = p2; back_i++;
      bm->back->verticies[back_i] = p3; back_i++;
      
    } else if ((!point_behind_plane(p1, bm->division)) &&
               (!point_behind_plane(p2, bm->division)) &&
               (!point_behind_plane(p3, bm->division))) {
               
      bm->front->triangle_normals[front_i / 3] = norm;
      bm->front->verticies[front_i] = p1; front_i++;
      bm->front->verticies[front_i] = p2; front_i++;
      bm->front->verticies[front_i] = p3; front_i++;
      
    } else {
      
      bm->back->triangle_normals[back_i / 3] = norm;
      bm->back->verticies[back_i] = p1; back_i++;
      bm->back->verticies[back_i] = p2; back_i++;
      bm->back->verticies[back_i] = p3; back_i++;
      bm->front->triangle_normals[front_i / 3] = norm;
      bm->front->verticies[front_i] = p1; front_i++;
      bm->front->verticies[front_i] = p2; front_i++;
      bm->front->verticies[front_i] = p3; front_i++;
      
    }
    
  }
  
  free(bm->verticies);
  free(bm->triangle_normals);
  
  bm->is_leaf = false;
  bm->verticies = NULL;
  bm->triangle_normals = NULL;
  bm->num_verticies = 0;
  
  bsp_mesh_subdivide(bm->front, iterations-1);
  bsp_mesh_subdivide(bm->back, iterations-1);
  
}

bounding_sphere bsp_mesh_bounding_sphere(bsp_mesh* bm) {
  
  if (bm->is_leaf) {
    
    bounding_box bb = bsp_mesh_bounding_box(bm);
    vector3 center;
    center.x = (bb.x_min + bb.x_max) / 2;
    center.y = (bb.y_min + bb.y_max) / 2;
    center.z = (bb.z_min + bb.z_max) / 2;
    
    float dist = 0;
    for(int i = 0; i < bm->num_verticies; i++) {
      dist = max(dist, v3_dist(bm->verticies[i], center));
    }
    
    bounding_sphere bs;
    bs.center = center;
    bs.radius = dist;
    bs.radius_sqrd = dist * dist;
    
    return bs;
    
  } else {
    bounding_sphere front_bs = bsp_mesh_bounding_sphere(bm->front);
    bounding_sphere back_bs = bsp_mesh_bounding_sphere(bm->back);
    return bounding_sphere_merge(front_bs, back_bs);
  }
  
}

bounding_box bsp_mesh_bounding_box(bsp_mesh* bm) {

  if (bm->is_leaf) {
    
    bounding_box bb = {10000, -10000, 10000, -10000, 10000, -10000};
    
    for(int i = 0; i < bm->num_verticies; i++) {
      bb.x_min = min(bb.x_min, bm->verticies[i].x);
      bb.x_max = max(bb.x_max, bm->verticies[i].x);
      bb.y_min = min(bb.y_min, bm->verticies[i].y);
      bb.y_max = max(bb.y_max, bm->verticies[i].y);
      bb.z_min = min(bb.z_min, bm->verticies[i].z);
      bb.z_max = max(bb.z_max, bm->verticies[i].z);
    }
    
    return bb;
    
  } else {
    bounding_box front_box = bsp_mesh_bounding_box(bm->front);
    bounding_box back_box = bsp_mesh_bounding_box(bm->back);
    return bounding_box_merge(front_box, back_box);
  }

}

void bsp_mesh_set_world_matrix(bsp_mesh* bm, matrix_4x4 world) {
  
  if (bm->is_leaf) {
    
    bm->world_matrix = world;
    
  } else {
    
    bm->world_matrix = world;
    bsp_mesh_set_world_matrix(bm->front, world);
    bsp_mesh_set_world_matrix(bm->back, world);
  
  }

}

collision_body* collision_body_new_sphere(vector3 center, float radius) {
  
  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_sphere;
  cb->collision_sphere.center = center;
  cb->collision_sphere.radius = radius;
  cb->collision_sphere.radius_sqrd = radius * radius;
  cb->collision_box = bounding_box_sphere(center, radius);
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

void collision_body_delete(collision_body* cb) {
  
  if (cb->collision_mesh != NULL) {
    bsp_mesh_delete(cb->collision_mesh);
  }
  free(cb);
  
}

void collision_body_set_world_matrix(collision_body* cb, matrix_4x4 world) {
  
  cb->world_matrix = world;
  
  if (cb->collision_type == collision_type_mesh) {
    bsp_mesh_set_world_matrix(cb->collision_mesh, world);
  }
}

collision_body* col_load_file(char* filename) {
  
  bsp_mesh* mesh = malloc(sizeof(bsp_mesh));
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
  
  bsp_mesh_subdivide(mesh, 5);
  
  collision_body* cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_mesh;
  cb->collision_mesh = mesh;
  cb->collision_sphere = bsp_mesh_bounding_sphere(cb->collision_mesh);
  cb->collision_box = bsp_mesh_bounding_box(cb->collision_mesh);
  
  return cb;
}

collision_info collision_info_merge(collision_info ci1, collision_info ci2) {
  
  collision_info ci;
  
  int i, j;
  for(i = 0; i < ci1.collisions; i++) {
    ci.position[i] = ci1.position[i];
    ci.normal[i] = ci1.normal[i];
  }
  
  for(j = i; j < MAX_COLLISIONS; j++) {
    ci.position[j] = ci2.position[j-i];
    ci.normal[j] = ci2.normal[j-i];
  }
  
  ci.collisions = max(ci1.collisions, ci2.collisions);
  return ci;
  
}

collision_info sphere_collide_sphere(bounding_sphere bs1, bounding_sphere bs2) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  if (v3_dist_sqrd(bs1.center, bs2.center) < bs1.radius_sqrd + bs2.radius_sqrd) {
    ci.collisions = 1;
    ci.position[0] = v3_div(v3_add(bs1.center, bs2.center), 2);
    ci.normal[0] = v3_normalize( v3_sub(bs1.center, bs2.center) );
  }
  
  return ci;

}

collision_info sphere_collide_box(bounding_sphere bs, bounding_box bb) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("sphere_collide_box unimplemented");
  
  return ci;

}

collision_info sphere_collide_mesh(bounding_sphere bs, bsp_mesh* bm) {
  
  if (!bm->is_leaf) {
    plane division = plane_transform(bm->division, bm->world_matrix);
    
    vector3 to_center = v3_sub(bs.center, division.position);
    float dist = v3_dot(to_center, division.direction);
    
    /* Intersects plane. Check both sides */
    if (fabs(dist) < bs.radius) {
      collision_info ci1 = sphere_collide_mesh(bs, bm->back);
      collision_info ci2 = sphere_collide_mesh(bs, bm->front);
      return collision_info_merge(ci1, ci2);
    }
    
    if (dist < 0) {
      return sphere_collide_mesh(bs, bm->back);
    } else {
      return sphere_collide_mesh(bs, bm->front);
    }
    
  }
  
  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  for(int i = 0; i < bm->num_verticies / 3; i++) {
    
    vector3 p1 = m44_mul_v3(bm->world_matrix, bm->verticies[i*3+0]);
    vector3 p2 = m44_mul_v3(bm->world_matrix, bm->verticies[i*3+1]);
    vector3 p3 = m44_mul_v3(bm->world_matrix, bm->verticies[i*3+2]);
    
    matrix_4x4 normworld = bm->world_matrix;
    normworld.xw = 0; normworld.yw = 0; normworld.zw = 0;
    
    vector3 norm = m44_mul_v3(normworld, bm->triangle_normals[i]);
    
    /* Offset to sphere center */
    p1 = v3_sub(p1, bs.center);
    p2 = v3_sub(p2, bs.center);
    p3 = v3_sub(p3, bs.center);
    
    /* Check intersection with triangle plane */
    bool plane_sep = fabs(v3_dot(p1, norm)) > bs.radius;
    if (plane_sep) continue;
    
    /* Check intersection on triangle verts */
    float p1p1 = v3_dot(p1, p1); float p1p2 = v3_dot(p1, p2); float p1p3 = v3_dot(p1, p3);
    float p2p2 = v3_dot(p2, p2); float p2p3 = v3_dot(p2, p3); float p3p3 = v3_dot(p3, p3);
    
    bool vert_sep1 = (p1p1 > bs.radius_sqrd) && (p1p2 > p1p1) && (p1p3 > p1p1);
    bool vert_sep2 = (p2p2 > bs.radius_sqrd) && (p1p2 > p2p2) && (p2p3 > p2p2);
    bool vert_sep3 = (p3p3 > bs.radius_sqrd) && (p1p3 > p3p3) && (p2p3 > p3p3);
    
    if (vert_sep1 || vert_sep2 || vert_sep3) continue;
    
    /* Check intersection on triangle edges */
    vector3 p1_p2 = v3_sub(p2, p1);
    vector3 p2_p3 = v3_sub(p3, p2);
    vector3 p3_p1 = v3_sub(p1, p3);
    float d1 = p1p2 - p1p1; float e1 = v3_dot(p1_p2, p1_p2);
    float d2 = p2p3 - p2p2; float e2 = v3_dot(p2_p3, p2_p3);
    float d3 = p1p3 - p3p3; float e3 = v3_dot(p3_p1, p3_p1);
    vector3 q1 = v3_sub(v3_mul(p1, e1) , v3_mul(p1_p2, d1));
    vector3 q2 = v3_sub(v3_mul(p2, e2) , v3_mul(p2_p3, d2));
    vector3 q3 = v3_sub(v3_mul(p3, e3) , v3_mul(p3_p1, d3));
    vector3 qp3 = v3_sub(v3_mul(p3, e1) , q1);
    vector3 qp1 = v3_sub(v3_mul(p1, e2) , q2);
    vector3 qp2 = v3_sub(v3_mul(p2, e3) , q3);
    bool edge_sep1 = (v3_dot(q1, q1) > bs.radius_sqrd * e1 * e1) && (v3_dot(q1, qp3) > 0);
    bool edge_sep2 = (v3_dot(q2, q2) > bs.radius_sqrd * e2 * e2) && (v3_dot(q2, qp1) > 0);
    bool edge_sep3 = (v3_dot(q3, q3) > bs.radius_sqrd * e3 * e3) && (v3_dot(q3, qp2) > 0);
    
    if (edge_sep1 || edge_sep2 || edge_sep3) continue;
        
    ci.position[ci.collisions] = p1; /* TODO: This isn't actual collision point. */
    ci.normal[ci.collisions] = norm; /* This is sometimes the opposide of the collision normal */
    ci.collisions++;
    
    if (ci.collisions == MAX_COLLISIONS) break;
  }
  
  return ci;
  
}

collision_info box_collide_sphere(bounding_box bb, bounding_sphere bs) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("box_collide_sphere unimplemented");
  
  return ci;

}

collision_info box_collide_box(bounding_box bb1, bounding_box bb2) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("box_collide_box unimplemented");
  
  return ci;

}

collision_info box_collide_mesh(bounding_box bb, bsp_mesh* bm) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("box_collide_mesh unimplemented");
  
  return ci;

}

collision_info mesh_collide_sphere(bsp_mesh* bm, bounding_sphere bs) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("mesh_collide_sphere unimplemented");
  
  return ci;

}

collision_info mesh_collide_box(bsp_mesh* bm, bounding_box bb) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("mesh_collide_box unimplemented");
  
  return ci;

}

collision_info mesh_collide_mesh(bsp_mesh* bm1, bsp_mesh* bm2) {

  collision_info ci;
  memset(&ci, 0, sizeof(collision_info));
  
  error("mesh_collide_mesh unimplemented");
  
  return ci;

}

collision_info collision_bodies_collide(collision_body* cb1, collision_body* cb2) {
  
  bounding_sphere sphere1 = bounding_sphere_transform(cb1->collision_sphere, cb1->world_matrix);
  bounding_sphere sphere2 = bounding_sphere_transform(cb2->collision_sphere, cb2->world_matrix);
  
  /* We first get collision info between bounding spheres */
  collision_info sphere_info = sphere_collide_sphere(sphere1, sphere2);
  
  /* If this doesn't collide we can be sure none of the others will */
  if (sphere_info.collisions == 0) { return sphere_info; }
  
  /* If both are spheres return immediately */
  if ((cb1->collision_type == collision_type_sphere) &&
      (cb2->collision_type == collision_type_sphere)) {
    return sphere_info;
  }
  
  bsp_mesh* mesh1 = cb1->collision_mesh;
  bsp_mesh* mesh2 = cb2->collision_mesh;
  
  /* Four special cases for sphere on mesh-box */
  /* These collisions are little use in the general case */
  /* So just return in these particular instances */
  if ((cb1->collision_type == collision_type_sphere) &&
      (cb2->collision_type == collision_type_mesh)) {
    return sphere_collide_mesh(sphere1, mesh2);
  }
  if ((cb1->collision_type == collision_type_mesh) &&
      (cb2->collision_type == collision_type_sphere)) {
    return mesh_collide_sphere(mesh1, sphere2);
  }
  
  bounding_box box1 = bounding_box_transform(cb1->collision_box, cb1->world_matrix);
  bounding_box box2 = bounding_box_transform(cb2->collision_box, cb2->world_matrix);
  
  if ((cb1->collision_type == collision_type_box) &&
      (cb2->collision_type == collision_type_sphere)) {
    return box_collide_sphere(box1, sphere2);
  }
  if ((cb1->collision_type == collision_type_sphere) &&
      (cb2->collision_type == collision_type_box)) {
    return sphere_collide_box(sphere1, box2);
  }
  
  /* We then get the bounding box info */
  collision_info box_info = box_collide_box(box1, box2);
  
  /* Again, if this doesn't collide the rest wont */
  if (box_info.collisions == 0) { return box_info; }
  
  /* If both are boxes return immediately */
  if ((cb1->collision_type == collision_type_box) &&
      (cb2->collision_type == collision_type_box)) {
    return box_info;
  }
  
  /* Twp more less interesting cases for mesh-box */
  if ((cb1->collision_type == collision_type_mesh) &&
      (cb2->collision_type == collision_type_box)) {
    return mesh_collide_box(mesh1, box2);
  }
  if ((cb1->collision_type == collision_type_box) &&
      (cb2->collision_type == collision_type_mesh)) {
    return box_collide_mesh(box1, mesh2);
  }

  /* Finally interesting case for mesh on mesh */
  return mesh_collide_mesh(mesh1, mesh2);
  
}

