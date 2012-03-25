#include <stdlib.h>
#include <math.h>

#include "SDL/SDL_local.h"

#include "error.h"
#include "matrix.h"
#include "int_list.h"
#include "vertex_list.h"
#include "vertex_hashtable.h"

#include "assets/collision_body.h"

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
  
  box bb = collision_mesh_box(cm);
  
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

sphere collision_mesh_sphere(collision_mesh* cm) {
  
  if (cm->is_leaf) {
    
    box bb = collision_mesh_box(cm);
    vector3 center;
    center.x = (bb.left.position.x + bb.right.position.x) / 2;
    center.y = (bb.top.position.y + bb.bottom.position.y) / 2;
    center.z = (bb.front.position.z + bb.back.position.z) / 2;
    
    float dist = 0;
    for(int i = 0; i < cm->num_verticies; i++) {
      dist = max(dist, v3_dist(cm->verticies[i], center));
    }
    
    sphere bs;
    bs.center = center;
    bs.radius = dist;
    bs.radius_sqrd = dist * dist;
    
    return bs;
    
  } else {
    sphere front_bs = collision_mesh_sphere(cm->front);
    sphere back_bs = collision_mesh_sphere(cm->back);
    return sphere_merge(front_bs, back_bs);
  }
  
}

box collision_mesh_box(collision_mesh* cm) {

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
    
    return box_new(x_min, x_max, y_min, y_max, z_min, z_max);
    
  } else {
    box front_box = collision_mesh_box(cm->front);
    box back_box = collision_mesh_box(cm->back);
    return box_merge(front_box, back_box);
  }

}

collision_body* collision_body_new_sphere(sphere bs) {
  
  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_sphere;
  cb->collision_sphere = bs;
  cb->collision_box = box_sphere(bs.center, bs.radius);
  cb->collision_mesh = NULL;
  
  return cb;
  
}

collision_body* collision_body_new_box(box bb) {

  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_box;
  cb->collision_sphere = sphere_of_box(bb);
  cb->collision_box = bb;
  cb->collision_mesh = NULL;

  return cb;
}

collision_body* collision_body_new_mesh(collision_mesh* cm) {

  collision_body*  cb = malloc(sizeof(collision_body));
  cb->collision_type = collision_type_mesh;
  cb->collision_sphere = collision_mesh_sphere(cm);
  cb->collision_box = collision_mesh_box(cm);
  cb->collision_mesh = cm;

  return cb;
}

void collision_body_delete(collision_body* cb) {
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

static float best_collision_time(float t0, float t1, float timestep) {
  float time = 0;
  if ((t0 >= 0) && (t0 <= timestep) &&
      (t1 >= 0) && (t1 <= timestep)) {
    time = min(t0, t1);
  } else if ((t0 >= 0) && (t0 <= timestep)) {
    time = t0;
  } else if ((t1 >= 0) && (t1 <= timestep))  {
    time = t1;
  }
  return time;
}

void sphere_collide_sphere(collision* out, sphere object, vector3 object_velocity, sphere target, float timestep) {
  
  float r = object.radius + target.radius;
  
  vector3 V = object_velocity;
  vector3 O = v3_sub(object.center, target.center);
  
  float A = v3_dot(V, V);
  float B = 2 * v3_dot(V, O);
  float C = v3_dot(O, O) - (r*r);
  
  float descrim = B*B - 4*A*C;
  if (descrim < 0) {
    return;
  }
  
  float dist_sqrd = sqrtf(descrim);
  
  /* This intermediate is used to reduce numerical instability */
  float q = (B < 0) ? (-B - dist_sqrd)/2.0 : (-B + dist_sqrd)/2.0;
  
  float t0 = q / A;
  float t1 = C / q;
  
  /* If outside of time range we don't care about it this frame */
  if (((t0 < 0) || (t0 > timestep)) &&
      ((t1 < 0) || (t1 > timestep))) {
    return;
  }
  
  float time = best_collision_time(t0, t1, timestep);
  
  if (time < out->time) {
    out->collided = true;
    out->time = time;
    out->object_position = v3_add(object.center, v3_mul(object_velocity, time - 0.001));
    out->surface_normal = v3_normalize(v3_sub(out->object_position, target.center));
    out->surface_position = v3_add(target.center, v3_mul(out->surface_normal, target.radius));
  }
  
}

void sphere_collide_box(collision* out, sphere object, vector3 object_velocity, box target, float timestep) {
  
  
  

}
    
static bool point_in_triangle(vector3 point, vector3 normal, vector3 v0, vector3 v1, vector3 v2) {
  
  vector3 d0 = v3_sub(v2, v0);
  vector3 d1 = v3_sub(v1, v0);
  vector3 d2 = v3_sub(point, v0);

  float dot00 = v3_dot(d0, d0);
  float dot01 = v3_dot(d0, d1);
  float dot02 = v3_dot(d0, d2);
  float dot11 = v3_dot(d1, d1);
  float dot12 = v3_dot(d1, d2);

  float inv_dom = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * inv_dom;
  float v = (dot00 * dot12 - dot01 * dot02) * inv_dom;

  return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

static void sphere_collide_vertex(collision* out, sphere object, vector3 object_velocity, vector3 vertex, float timestep) {
  
  vector3 O = v3_sub(object.center, vertex);
  vector3 V = object_velocity;
  float r_r = object.radius_sqrd;
  
  float A = v3_dot(V, V);
  float B = 2 * v3_dot(V, O);
  float C = v3_dot(O, O) - r_r;
  
  float descrim = B*B - 4*A*C;
  if (descrim < 0) {
    return;
  }
  
  float dist_sqrd = sqrtf(descrim);
  
  /* This intermediate is used to reduce numerical instability */
  float q = (B < 0) ? (-B - dist_sqrd)/2.0 : (-B + dist_sqrd)/2.0;
  
  float t0 = q / A;
  float t1 = C / q;
  
  if (((t0 < 0) || (t0 > timestep)) &&
      ((t1 < 0) || (t1 > timestep))) {
    return;
  }
  
  float time = best_collision_time(t0, t1, timestep);
  
  if (time < out->time) {
    out->collided = true;
    out->time = time;
    out->object_position = v3_add(object.center, v3_mul(object_velocity, time - 0.001));;
    out->surface_normal = v3_normalize(v3_sub(object.center, vertex));
    out->surface_position = v3_add(out->object_position, v3_mul(out->surface_normal, -object.radius));;
  }
  
}

static void sphere_collide_edge(collision* out, sphere object, vector3 object_velocity, vector3 v0, vector3 v1, float timestep) {
  
  float r_r = object.radius * object.radius;
  vector3 O = v3_sub(object.center, object.center);
  v0 = v3_sub(v0, object.center);
  v1 = v3_sub(v1, object.center);
  
  vector3 E = v3_sub(v1, v0);
  vector3 V = object_velocity;
  vector3 X = v3_sub(v0, O);
  
  float A = v3_length_sqrd(E) * -v3_length_sqrd(V) + v3_dot(E, V) * v3_dot(E, V);
  float B = v3_length_sqrd(E) * 2 * v3_dot(V, X) - 2 * v3_dot(E, V) * v3_dot(E, X);
  float C = v3_length_sqrd(E) * (r_r - v3_length_sqrd(X)) + v3_dot(E, X) * v3_dot(E, X);
  
  float descrim = B*B - 4*A*C;
  if (descrim < 0) {
    return;
  }
  
  float dist_sqrd = sqrtf(descrim);
  
  /* This intermediate is used to reduce numerical instability */
  float q = (B < 0) ? (-B - dist_sqrd)/2.0 : (-B + dist_sqrd)/2.0;
  
  float t0 = q / A;
  float t1 = C / q;
  
  if (((t0 < 0) || (t0 > timestep)) &&
      ((t1 < 0) || (t1 > timestep))) {
    return;
  }
  
  float time = best_collision_time(t0, t1, timestep);
  
  float range = (v3_dot(E, V) * time - v3_dot(E, X)) / v3_length_sqrd(E);
  if ((range < 0) || (range > 1)) return;
  
  if (time < out->time) {
    out->collided = true;
    out->time = time;
    out->object_position = v3_add(object.center, v3_mul(object_velocity, time - 0.001));
    out->surface_position = v3_add(v0, v3_mul(E, range));
    
    /* Not sure how accurate this is */
    vector3 incident = v3_sub(out->surface_position, out->object_position);
    vector3 outward = v3_cross(incident, E);
    vector3 normal = v3_cross(outward, E);
    
    out->surface_normal = v3_normalize(normal);
  }
  
}

static const int before = 0; 
static const int behind = 1; 
static const int intersecting = 2; 

static int sphere_plane_location(sphere object, vector3 object_velocity, plane p, float timestep) {
  
  float angle = v3_dot(p.direction, object_velocity);
  float dist = v3_dot(p.direction, v3_sub(object.center, p.position)); 
  
  float t0 = ( object.radius - dist) / angle;
  float t1 = (-object.radius - dist) / angle;
  
  if (max(t0, t1) < 0) {
    return (angle < 0) ? behind : before;
  }
  if (min(t0, t1) > timestep) {
    return (angle < 0) ? before : behind;
  }
  
  return intersecting;
  
}

void sphere_collide_mesh(collision* out, sphere object, vector3 object_velocity, collision_mesh* target, matrix_4x4 target_world, float timestep) {
  
  if (!target->is_leaf) {
    
    plane division = plane_transform(target->division, target_world);
    int location = sphere_plane_location(object, object_velocity, division, timestep);
    
    if (location == before) {
      sphere_collide_mesh(out, object, object_velocity, target->front, target_world, timestep);
      return;
    }
    if (location == behind) {
      sphere_collide_mesh(out, object, object_velocity, target->back, target_world, timestep);
      return;
    }
    if (location == intersecting) {
      sphere_collide_mesh(out, object, object_velocity, target->front, target_world, timestep);
      sphere_collide_mesh(out, object, object_velocity, target->back, target_world, timestep);
      return;
    }

  }
  
  for(int i=0; i < target->num_verticies / 3; i++) {
    
    vector3 v0 = target->verticies[i*3+0];
    vector3 v1 = target->verticies[i*3+1];
    vector3 v2 = target->verticies[i*3+2];
        
    v0 = m44_mul_v3(target_world, v0);
    v1 = m44_mul_v3(target_world, v1);
    v2 = m44_mul_v3(target_world, v2);
    
    vector3 norm = target->triangle_normals[i];
    matrix_4x4 norm_world = target_world;
    norm_world.xw = 0; norm_world.yw = 0; norm_world.zw = 0;
    
    norm = m44_mul_v3(norm_world, norm);
    norm = v3_normalize(norm);
    
    float angle = v3_dot(norm, object_velocity);
    float dist = v3_dot(norm, v3_sub(object.center, v0)); 
    
    float t0 = ( object.radius - dist) / angle;
    float t1 = (-object.radius - dist) / angle;
    
    if (((t0 > 0) && (t0 < timestep)) ||
        ((t1 > 0) && (t1 < timestep))) {
    
      /* Test plane collision inside triangle */
      
      float time = best_collision_time(t0, t1, timestep);
      
      vector3 collision_point = v3_add(object.center, v3_mul(object_velocity, time - 0.001));
      vector3 surface_point = v3_add(collision_point, v3_mul(norm, -object.radius));
      
      if ((time < out->time) && point_in_triangle(surface_point, norm, v0, v1, v2)) {
        out->collided = true;
        out->time = time;
        out->object_position = collision_point;
        out->surface_normal = norm;
        out->surface_position = surface_point;
        continue;
      }
    
    }
    
    /* Sweep test */
    
    sphere_collide_edge(out, object, object_velocity, v0, v1, timestep);
    sphere_collide_edge(out, object, object_velocity, v1, v2, timestep);
    sphere_collide_edge(out, object, object_velocity, v2, v0, timestep);
    
    sphere_collide_vertex(out, object, object_velocity, v0, timestep);
    sphere_collide_vertex(out, object, object_velocity, v1, timestep);
    sphere_collide_vertex(out, object, object_velocity, v2, timestep);
    
  }
  
  
}
