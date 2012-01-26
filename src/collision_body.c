#include <stdlib.h>

#include "SDL/SDL_RWops.h"

#include "error.h"
#include "matrix.h"
#include "int_list.h"
#include "vertex_list.h"
#include "vertex_hashtable.h"

#include "collision_body.h"

bool point_behind_plane(vector3 point, plane plane) {
  
  vector3 to_point = v3_sub(point, plane.position);
  float angle = v3_dot(to_point, plane.direction);
  
  if (angle < 0) {
    return true;
  } else {
    return false;
  }
  
}

bool bounding_box_intersects(bounding_box b1, bounding_box b2) {
  error("bounding_box_intersects Not implemented yet!");
  return false;
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

bool bounding_sphere_intersects(bounding_sphere bs1, bounding_sphere bs2) {
  error("bounding_sphere_intersects not implemented yet!");
  return false;
}

bool bounding_sphere_contains(bounding_sphere bs1, vector3 point) {
  float dist_sqrt = v3_dist_sqrd(bs1.center, point);
  return dist_sqrt <= bs1.radius_sqrd;
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

void collision_body_delete(collision_body* cb) {
  
  bsp_mesh_delete(cb->collision_mesh);
  free(cb);
  
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
    vertex v2 = vertex_list_get(vert_list, i1);
    vertex v3 = vertex_list_get(vert_list, i1);
    
    vector3 norm = triangle_normal(v1, v2, v3);
    
    mesh->verticies[i*3+0] = v1.position;
    mesh->verticies[i*3+1] = v2.position;
    mesh->verticies[i*3+2] = v3.position;
    
    mesh->triangle_normals[i] = norm;
  }
  
  vertex_hashtable_delete(vert_hashes);
  vertex_list_delete(vert_data);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  
  bsp_mesh_subdivide(mesh, 10);
  
  collision_body* cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_mesh;
  cb->collision_mesh = mesh;
  cb->collision_sphere = bsp_mesh_bounding_sphere(cb->collision_mesh);
  cb->collision_box = bsp_mesh_bounding_box(cb->collision_mesh);
  
  return cb;
}