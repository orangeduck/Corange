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
  bb.z_max = max(b1.y_max, b2.z_max);
  
  return bb;
}

plane bounding_box_division(bounding_box bb) {
  
  float x_diff = bb.x_max - bb.x_min;
  float y_diff = bb.y_max - bb.y_min;
  float z_diff = bb.z_max - bb.z_min;
  
  plane p;
  p.position.x = (bb.x_min + bb.x_min) / 2;
  p.position.y = (bb.y_min + bb.y_min) / 2;
  p.position.z = (bb.z_min + bb.z_min) / 2;
  
  if ((x_diff >= y_diff) && (x_diff >= z_diff)) {
    p.direction = v3(1,0,0);
  } else if ((y_diff >= x_diff) && (y_diff >= z_diff)) {
    p.direction = v3(0,1,0); 
  } else if ((z_diff >= x_diff) && (z_diff >= y_diff)) {
    p.direction = v3(0,0,1);
  }
  
  return p;
  
}

void bsp_mesh_delete(bsp_mesh* bm) {
  
  if (bm->is_leaf) {
    free(bm->verticies);
    free(bm->triangles);
    free(bm);
  } else {
    bsp_mesh_delete(bm->front);
    bsp_mesh_delete(bm->back);
    free(bm);
  }
  
}

void bsp_mesh_subdivide(bsp_mesh* bm, int iterations) {

}

float bsp_mesh_bounding_distance(bsp_mesh* bm) {
  
  if (bm->is_leaf) {
    
    float dist = 0;
    int i;
    for(i = 0; i < bm->num_verticies; i++) {
      dist = max(dist, v3_length(bm->verticies[i]));
    }
    
    return dist;
    
  } else {
    float front_dist = bsp_mesh_bounding_distance(bm->front);
    float back_dist = bsp_mesh_bounding_distance(bm->back);
    return max(front_dist, back_dist);
  }
  
}

bounding_box bsp_mesh_bounding_box(bsp_mesh* bm) {

  if (bm->is_leaf) {
    
    bounding_box bb = {10000, -10000, 10000, -10000, 10000, -10000};
    int i;
    for(i = 0; i < bm->num_verticies; i++) {
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

static vector3 baricentric_coordinates(vector3 p1, vector3 p2, vector3 p3, vector3 pos) {

	float b_top = (p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.z - p3.z);
	float b_bot = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	
	float y_top = (p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.z - p3.z);
	float y_bot = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	
	float b = b_top / b_bot;
	float y = y_top / y_bot;
	float a = 1 - b - y;
	
	return v3(a, b, y);
}

vector3 bsp_mesh_ground_point(bsp_mesh* bm, vector3 point) {
  
  if (bm->is_leaf) {
    
    float highest_y = 0;
    
    int i;
    for(i = 0; i < bm->num_triangles; i++) {
      
      int i1 = bm->triangles[i*3+0];
      int i2 = bm->triangles[i*3+1];
      int i3 = bm->triangles[i*3+2];
      
      vector3 p1 = bm->verticies[i1];
      vector3 p2 = bm->verticies[i2];
      vector3 p3 = bm->verticies[i3];
      
      /* Only search triangles below the point */
      float y_min = min(min(p1.y, p2.y), p3.y);
      if (y_min >= point.y) {
        continue;
      }
      
      float x_min = min(min(p1.x, p2.x), p3.x);
      float x_max = max(max(p1.x, p2.x), p3.x);
      float z_min = min(min(p1.z, p2.z), p3.z);
      float z_max = max(max(p1.z, p2.z), p3.z);
    
      if ((point.x <= x_max) &&
          (point.x >= x_min) &&
          (point.z <= z_max) &&
          (point.z >= z_min)) {
        
        vector3 bari = baricentric_coordinates(p1, p2, p3, point);
        if ((bari.x >= 0.0) && (bari.x <= 1.0) && 
            (bari.y >= 0.0) && (bari.y <= 1.0) && 
            (bari.z >= 0.0) && (bari.z <= 1.0)) {
          
          /* Use the highest y which is below the point */
          float triangle_y = bari.x * p1.y + bari.y * p2.y + bari.z * p3.y;
          highest_y = max(highest_y, triangle_y);
        }
      }
    }
    
    if (highest_y == 0) {
      debug("Default to 0");
    }
    return v3(point.x, highest_y, point.z);
  
  } else {
    if (point_behind_plane(point, bm->division)) {
      return bsp_mesh_ground_point(bm->back, point);
    } else {
      return bsp_mesh_ground_point(bm->front, point);
    }
  }

}

void collision_body_delete(collision_body* cb) {
  
  bsp_mesh_delete(cb->collision_mesh);
  free(cb);
  
}

vector3 collision_body_ground_point(collision_body* cb, matrix_4x4 world_matrix, vector3 point) {

  matrix_4x4 world_inv = m44_inverse(world_matrix);
  point = m44_mul_v3(world_inv, point);
  
  if (v3_length(point) > cb->bounding_distance) {
    return m44_mul_v3(world_matrix, point);
  }
  
  if (!bounding_box_contains(cb->bounding_box, point)) {
    return m44_mul_v3(world_matrix, point);
  }
  
  point = bsp_mesh_ground_point(cb->collision_mesh, point);
  return m44_mul_v3(world_matrix, point);
  
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

  mesh->num_verticies = vert_index;
  mesh->num_triangles = tri_list->num_items / 3;
  
  int i;
  
  mesh->verticies = malloc(sizeof(vector3) * mesh->num_verticies);
  for(i = 0; i < mesh->num_verticies; i++) {
    mesh->verticies[i] = vertex_list_get(vert_list, i).position;
  }
  
  mesh->triangles = malloc(sizeof(int) * mesh->num_triangles * 3);
  for(i = 0; i < mesh->num_triangles * 3; i++) {
    mesh->triangles[i] = int_list_get(tri_list, i);
  }
  
  vertex_hashtable_delete(vert_hashes);
  vertex_list_delete(vert_data);
  vertex_list_delete(vert_list);
  int_list_delete(tri_list);
  
  collision_body* cb = malloc(sizeof(collision_body));
  cb->collision_mesh = mesh;
  cb->bounding_distance = bsp_mesh_bounding_distance(cb->collision_mesh);
  cb->bounding_box = bsp_mesh_bounding_box(cb->collision_mesh);
  
  return cb;
}