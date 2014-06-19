#include "assets/cmesh.h"

#include "data/vertex_list.h"

static sphere ctri_bound(ctri t) {
  
  vec3 center = vec3_div(vec3_add(vec3_add(t.a, t.b), t.c), 3);
  
  float radius = 0;
  radius = max(radius, vec3_dist_sqrd(t.a, center));
  radius = max(radius, vec3_dist_sqrd(t.b, center));
  radius = max(radius, vec3_dist_sqrd(t.c, center));
  
  return sphere_new(center, sqrt(radius));
  
}

ctri ctri_new(vec3 a, vec3 b, vec3 c, vec3 norm) {
  ctri t;
  t.a = a;
  t.b = b;
  t.c = c;
  t.norm = norm;
  t.bound = sphere_unit();
  t.bound = ctri_bound(t);
  return t;
}

bool ctri_inside_plane(ctri t, plane p) {
  return (point_inside_plane(t.a, p) &&
          point_inside_plane(t.b, p) &&
          point_inside_plane(t.c, p));
}

bool ctri_outside_plane(ctri t, plane p) {
  return (point_outside_plane(t.a, p) &&
          point_outside_plane(t.b, p) &&
          point_outside_plane(t.c, p));
}

bool ctri_intersects_plane(ctri t, plane p) {
  return (!ctri_inside_plane(t, p) && !ctri_outside_plane(t, p));
}

ctri ctri_transform(ctri t, mat4 m, mat3 mn) {
  t.a = mat4_mul_vec3(m, t.a);
  t.b = mat4_mul_vec3(m, t.b);
  t.c = mat4_mul_vec3(m, t.c);
  t.norm  = vec3_normalize(mat3_mul_vec3(mn, t.norm));
  t.bound = ctri_bound(t);
  return t;
}

ctri ctri_transform_space(ctri t, mat3 s, mat3 sn) {
  t.a = mat3_mul_vec3(s, t.a);
  t.b = mat3_mul_vec3(s, t.b);
  t.c = mat3_mul_vec3(s, t.c);
  t.norm  = vec3_normalize(mat3_mul_vec3(sn, t.norm));
  t.bound = ctri_bound(t);
  return t;
}

void cmesh_delete(cmesh* cm) {
  
  if (cm->is_leaf) {
    free(cm->triangles);
  } else {
    cmesh_delete(cm->front);
    cmesh_delete(cm->back);
  }

  free(cm);
  
}

static vec3 cmesh_center(cmesh* cm) {

  if (!cm->is_leaf) {
    error("cmesh is not leaf type!");
    return vec3_zero();
  }

  if (cm->triangles_num == 0) {
    error("Can't find center of mesh with no triangles");
    return vec3_zero();
  }
  
  vec3 center = vec3_zero();
  for(int i = 0; i < cm->triangles_num; i++) {
    center = vec3_add(center, cm->triangles[i].a);
    center = vec3_add(center, cm->triangles[i].b);
    center = vec3_add(center, cm->triangles[i].c);
  }
  center = vec3_div(center, cm->triangles_num * 3);
  
  return center;

}

static float cmesh_radius(cmesh* cm) {
  
  if (!cm->is_leaf) {
    error("cmesh is not leaf type!");
    return 0;
  }
  
  vec3 center = cmesh_center(cm);
  float radius = 0;
  for (int i = 0; i < cm->triangles_num; i++) {
    radius = max(radius, vec3_dist(center, cm->triangles[i].a));
    radius = max(radius, vec3_dist(center, cm->triangles[i].b));
    radius = max(radius, vec3_dist(center, cm->triangles[i].c));
  }
  
  return radius;
  
}

static box cmesh_box(cmesh* cm) {

  if (!cm->is_leaf) {
    error("cmesh is not leaf type!");
    return box_new(0,0,0,0,0,0);
  }
  
  if (cm->triangles_num == 0) {
    error("Can't find box of mesh with no triangles");
    return box_new(0,0,0,0,0,0);
  }
  
  float x_min = 0; float x_max = 0;
  float y_min = 0; float y_max = 0;
  float z_min = 0; float z_max = 0;
  
  for(int i = 0; i < cm->triangles_num; i++) {
    x_min = min(x_min, cm->triangles[i].a.x);
    x_max = max(x_max, cm->triangles[i].a.x);
    y_min = min(y_min, cm->triangles[i].a.y);
    y_max = max(y_max, cm->triangles[i].a.y);
    z_min = min(z_min, cm->triangles[i].a.z);
    z_max = max(z_max, cm->triangles[i].a.z);
    
    x_min = min(x_min, cm->triangles[i].b.x);
    x_max = max(x_max, cm->triangles[i].b.x);
    y_min = min(y_min, cm->triangles[i].b.y);
    y_max = max(y_max, cm->triangles[i].b.y);
    z_min = min(z_min, cm->triangles[i].b.z);
    z_max = max(z_max, cm->triangles[i].b.z);
    
    x_min = min(x_min, cm->triangles[i].c.x);
    x_max = max(x_max, cm->triangles[i].c.x);
    y_min = min(y_min, cm->triangles[i].c.y);
    y_max = max(y_max, cm->triangles[i].c.y);
    z_min = min(z_min, cm->triangles[i].c.z);
    z_max = max(z_max, cm->triangles[i].c.z);
  }
  
  return box_new(x_min, x_max, y_min, y_max, z_min, z_max);

}

static plane cmesh_division(cmesh* cm) {
  
  box bb = cmesh_box(cm);
  
  plane p;
  p.position = cmesh_center(cm);
    
  float x_diff = bb.left.position.x  - bb.right.position.x;
  float y_diff = bb.top.position.y   - bb.bottom.position.y;
  float z_diff = bb.front.position.z - bb.back.position.z;
  
  if ((x_diff >= y_diff) && 
      (x_diff >= z_diff)) {
    p.direction = vec3_new(1,0,0);
    
  } else if ((y_diff >= x_diff) && 
             (y_diff >= z_diff)) {
    p.direction = vec3_new(0,1,0);
    
  } else if ((z_diff >= x_diff) && 
             (z_diff >= y_diff)) {
    p.direction = vec3_new(0,0,1);
  } else {
    p.direction = vec3_new(1,0,0);
  }
  
  return p;
  
}

sphere cmesh_bound(cmesh* cm) {
  
  if (!cm->is_leaf) {
    error("cmesh is not leaf type!");
    return sphere_new(vec3_zero(), 0);
  }
  
  return sphere_new(cmesh_center(cm), cmesh_radius(cm));
  
}

void cmesh_subdivide(cmesh* cm, int iterations) {
  
  if (!cm->is_leaf) {
    error("Attempt to subdivide non-leaf bsp tree!");
    return;
  }
  
  if (iterations == 0) { return; }
  if (cm->triangles_num < 10) { return; }
  
  plane division = cmesh_division(cm);
  
  int num_front = 0;
  int num_back  = 0;
  
  for(int i = 0; i < cm->triangles_num; i++) {
    ctri t = cm->triangles[i];
    
    if (ctri_inside_plane(t, division)) {
      num_back++;
    } else if (ctri_outside_plane(t, division)) {
      num_front++;
    } else { 
      num_back++; 
      num_front++; 
    }
  }
  
  if (num_front > cm->triangles_num * 0.75) { return; }
  if (num_back  > cm->triangles_num * 0.75) { return; }
  
  cmesh* front = malloc(sizeof(cmesh));
  front->is_leaf = true;
  front->triangles_num = num_front;
  front->triangles = malloc(sizeof(ctri) * num_front);
  
  cmesh* back = malloc(sizeof(cmesh));
  back->is_leaf = true;
  back->triangles_num = num_back;
  back->triangles = malloc(sizeof(ctri) * num_back);
  
  int i_front = 0;
  int i_back  = 0;
  
  for(int i = 0; i < cm->triangles_num; i++) {
    
    ctri t = cm->triangles[i];
    
    if (ctri_inside_plane(t, division)) {
      back->triangles[i_back] = t;
      i_back++;
    } else if (ctri_outside_plane(t, division)) {
      front->triangles[i_front] = t;
      i_front++;
    } else { 
      back->triangles[i_back] = t;
      front->triangles[i_front] = t;
      i_back++; 
      i_front++; 
    }
    
  }
  
  if (front->triangles_num > 0) {
    front->bound = cmesh_bound(front);
  } else {
    front->bound = sphere_new(vec3_zero(), 0);
  }
  
  if (back->triangles_num > 0) {
    back->bound = cmesh_bound(back);
  } else {
    back->bound = sphere_new(vec3_zero(), 0);
  }
  
  cmesh_subdivide(front, iterations-1);
  cmesh_subdivide(back, iterations-1);
  
  free(cm->triangles);
  cm->is_leaf = false;
  cm->division = division;
  cm->front = front;
  cm->back = back;
  
}

static int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDL_RWread(file, &c, 1, 1);
    
    if (status == -1) return -1;
    if (i == buffersize-1) return -1;
    if (status == 0) break;
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    buffer[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

cmesh* col_load_file(char* filename) {
    
  cmesh* cm = malloc(sizeof(cmesh));
  cm->is_leaf = true;
  
  vertex_list* vert_positions = vertex_list_new();
  vertex_list* vert_triangles = vertex_list_new();
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
    
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    float px, py, pz;
    if (sscanf(line, "v %f %f %f", &px, &py, &pz) == 3) {
      
      vertex vert = vertex_new();
      vert.position = vec3_new(px, py, pz);
      vertex_list_push_back(vert_positions, vert);
      
    }
    
    int pi1, ti1, ni1, pi2, ti2, ni2, pi3, ti3, ni3, pi4;
    
    if (sscanf(line, "f %i// %i// %i// %i//", &pi1, &pi2, &pi3, &pi4) == 4) {
      
      vertex a1 = vertex_list_get(vert_positions, pi1-1);
      vertex b1 = vertex_list_get(vert_positions, pi2-1);
      vertex c1 = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a1);
      vertex_list_push_back(vert_triangles, b1);
      vertex_list_push_back(vert_triangles, c1);
      
      vertex a2 = vertex_list_get(vert_positions, pi1-1);
      vertex b2 = vertex_list_get(vert_positions, pi3-1);
      vertex c2 = vertex_list_get(vert_positions, pi4-1);
      
      vertex_list_push_back(vert_triangles, a2);
      vertex_list_push_back(vert_triangles, b2);
      vertex_list_push_back(vert_triangles, c2);
      
    }
    
    else if (sscanf(line, "f %i/%i/%i %i/%i/%i %i/%i/%i", &pi1, &ti1, &ni1, &pi2, &ti2, &ni2, &pi3, &ti3, &ni3) == 9) {
      
      vertex a = vertex_list_get(vert_positions, pi1-1);
      vertex b = vertex_list_get(vert_positions, pi2-1);
      vertex c = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a);
      vertex_list_push_back(vert_triangles, b);
      vertex_list_push_back(vert_triangles, c);
      
    }
    
    else if (sscanf(line, "f %i//%i %i//%i %i//%i", &pi1, &ni1, &pi2, &ni2, &pi3, &ni3) == 6) {
      
      vertex a = vertex_list_get(vert_positions, pi1-1);
      vertex b = vertex_list_get(vert_positions, pi2-1);
      vertex c = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a);
      vertex_list_push_back(vert_triangles, b);
      vertex_list_push_back(vert_triangles, c);
      
    }
    
    else if (sscanf(line, "f %i/%i %i/%i %i/%i", &pi1, &ti1, &pi2, &ti2, &pi3, &ti3) == 6) {
      
      vertex a = vertex_list_get(vert_positions, pi1-1);
      vertex b = vertex_list_get(vert_positions, pi2-1);
      vertex c = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a);
      vertex_list_push_back(vert_triangles, b);
      vertex_list_push_back(vert_triangles, c);
      
    }
    
    else if (sscanf(line, "f %i %i %i", &pi1, &pi2, &pi3) == 3) {
      
      vertex a = vertex_list_get(vert_positions, pi1-1);
      vertex b = vertex_list_get(vert_positions, pi2-1);
      vertex c = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a);
      vertex_list_push_back(vert_triangles, b);
      vertex_list_push_back(vert_triangles, c);
      
    }
    
    else if (sscanf(line, "f %i// %i// %i//", &pi1, &pi2, &pi3) == 3) {
      
      vertex a = vertex_list_get(vert_positions, pi1-1);
      vertex b = vertex_list_get(vert_positions, pi2-1);
      vertex c = vertex_list_get(vert_positions, pi3-1);
      
      vertex_list_push_back(vert_triangles, a);
      vertex_list_push_back(vert_triangles, b);
      vertex_list_push_back(vert_triangles, c);
      
    }
    
  }
    
  SDL_RWclose(file);

  cm->triangles_num = vert_triangles->num_items / 3;
  cm->triangles = malloc(sizeof(ctri) * cm->triangles_num);
    
  for(int i = 0; i < vert_triangles->num_items; i += 3) {
    
    vertex a = vertex_list_get(vert_triangles, i+0);
    vertex b = vertex_list_get(vert_triangles, i+1);
    vertex c = vertex_list_get(vert_triangles, i+2);
    vec3 norm = triangle_normal(a, b, c);
    
    cm->triangles[i / 3] = ctri_new(a.position, b.position, c.position, norm);
    
  }
  
  vertex_list_delete(vert_positions);
  vertex_list_delete(vert_triangles);
  
  if (cm->triangles_num != 0) {
    cm->bound = cmesh_bound(cm);
  } else {
    cm->bound = sphere_new(vec3_zero(), 0);
  }
  
  cmesh_subdivide(cm, 5);
  
  return cm;
}
