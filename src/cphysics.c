#include "cphysics.h"

vec3 vec3_gravity() {
  return vec3_new(0, -9.81, 0);
}

bool quadratic(float a, float b, float c, float* t0, float* t1) {

  float descrim = b*b - 4*a*c;
  
  if (descrim < 0) {
  
    return false;
  
  } else {
    
    float d = sqrtf(descrim);
    float q = (b < 0) ? (-b - d) / 2.0 : (-b + d) / 2.0;
    
    *t0 = q / a;
    *t1 = c / q;
    
    return true;
  }

}

collision collision_none() {
  
  collision col;
  col.collided = false;
  col.time = FLT_MAX;
  col.point = vec3_zero();
  col.norm = vec3_zero();
  
  return col;
  
}

collision collision_new(float time, vec3 point, vec3 norm) {
  
  collision col;
  col.collided = true;
  col.time = time;
  col.point = point;
  col.norm = norm;
  
  return col;
}

collision collision_merge(collision c0, collision c1) {
  
  collision c;
  c.collided = c0.collided || c1.collided;
  c.time = min(c0.time, c1.time);
  c.point  = c0.time < c1.time ? c0.point : c1.point;
  c.norm   = c0.time < c1.time ? c0.norm  : c1.norm;
  
  return c;
}

collision sphere_collide_face(sphere s, vec3 v, ctri ct) {
  
  //if (unlikely(sphere_intersects_face(s, ct.a, ct.b, ct.c, ct.norm))) { error("Collision Sphere Inside Mesh Face!"); }
  
  if (vec3_dot(ct.norm, v) > 0) {
    ct.norm = vec3_neg(ct.norm);
  }
  
  float angle = vec3_dot(ct.norm, v);
  float dist  = vec3_dot(ct.norm, vec3_sub(s.center, ct.a)); 
  
  float t0 = ( s.radius - dist) / angle;
  float t1 = (-s.radius - dist) / angle;
  float t = FLT_MAX;
  
  if (between_or(t0, 0, 1) && between_or(t1, 0, 1)) { t = min(t0, t1); }
  else if (between_or(t0, 0, 1)) { t = t0; }
  else if (between_or(t1, 0, 1)) { t = t1; } 
  else { return collision_none(); }
  
  vec3 cpoint = vec3_add(s.center, vec3_mul(v, t));
  vec3 spoint = vec3_add(cpoint, vec3_mul(ct.norm, -s.radius));
  
  if (!point_inside_triangle(spoint, ct.a, ct.b, ct.c)) {
    return collision_none();
  } else {
    return collision_new(t, spoint, ct.norm);
  }
  
}

collision sphere_collide_edge(sphere s, vec3 v, vec3 e0, vec3 e1) {
  
  //if (unlikely(!line_outside_sphere(s, e0, e1))) { error("Collision Sphere Inside Mesh Edge!"); }
  
  vec3 x0 = vec3_sub(e0, s.center);
  vec3 x1 = vec3_sub(e1, s.center);
  
  vec3 d = vec3_sub(x1, x0);  
  float dlen = vec3_length_sqrd(d);
  float vlen = vec3_length_sqrd(v);
  float xlen = vec3_length_sqrd(x0);
  
  float A = dlen * -vlen + vec3_dot(d, v) * vec3_dot(d, v);
  float B = dlen * 2 * vec3_dot(v, x0) - 2 * vec3_dot(d, v) * vec3_dot(d, x0);
  float C = dlen * (s.radius * s.radius - xlen) + vec3_dot(d, x0) * vec3_dot(d, x0);
  
  float t0, t1, t;
  if (!quadratic(A, B, C, &t0, &t1)) { return collision_none(); }
  
  if (between_or(t0, 0, 1) && between_or(t1, 0, 1)) { t = min(t0, t1); }
  else if (between_or(t0, 0, 1)) { t = t0; }
  else if (between_or(t1, 0, 1)) { t = t1; } 
  else { return collision_none(); }
  
  float range = (vec3_dot(d, v) * t - vec3_dot(d, x0)) / dlen;
  
  if (!between_or(range, 0, 1)) {
    return collision_none();
  } else {
    vec3 spoint = vec3_add(e0, vec3_mul(d, range));
    return collision_new(t, spoint, vec3_normalize(vec3_sub(s.center, spoint)));
  }
  
}

collision sphere_collide_point(sphere s, vec3 v, vec3 p) {

  //if (unlikely(!point_outside_sphere(s, p))) { error("Collision Sphere Inside Mesh Vertex!"); }

  vec3  o = vec3_sub(s.center, p);
  float A = vec3_dot(v, v);
  float B = 2 * vec3_dot(v, o);
  float C = vec3_dot(o, o) - (s.radius * s.radius);
  
  float t0, t1, t;
  if (!quadratic(A, B, C, &t0, &t1)) { return collision_none(); }
  
  if (between_or(t0, 0, 1) && between_or(t1, 0, 1)) { t = min(t0, t1); }
  else if (between_or(t0, 0, 1)) { t = t0; }
  else if (between_or(t1, 0, 1)) { t = t1; } 
  else { return collision_none(); }
  
  return collision_new(t, p, vec3_normalize(vec3_sub(s.center, p)));
  
}

collision sphere_collide_sphere(sphere s, vec3 v, sphere s0) {

  //if (unlikely(!sphere_outside_sphere(s, s0))) { error("Collision Sphere Inside Sphere!"); }

  vec3  o = vec3_sub(s.center, s0.center);
  float A = vec3_dot(v, v);
  float B = 2 * vec3_dot(v, o);
  float C = vec3_dot(o, o) - ((s.radius + s0.radius) * (s.radius + s0.radius));
  
  float t0, t1, t;
  if (!quadratic(A, B, C, &t0, &t1)) { return collision_none(); }
  
  if (between_or(t0, 0, 1) && between_or(t1, 0, 1)) { t = min(t0, t1); }
  else if (between_or(t0, 0, 1)) { t = t0; }
  else if (between_or(t1, 0, 1)) { t = t1; } 
  else { return collision_none(); }
  
  vec3 proj = vec3_add(s.center, vec3_mul(v, t));
  vec3 twrd = vec3_normalize(vec3_sub(s0.center, proj));
  vec3 p = vec3_add(proj, vec3_mul(twrd, s.radius));
  
  return collision_new(t, p, vec3_normalize(vec3_sub(s.center, p)));

}

collision sphere_collide_ctri(sphere s, vec3 v, ctri ct) {
  
  if (!sphere_swept_intersects_plane(s, v, plane_new(ct.a, ct.norm))) {
    return collision_none();
  }
  
  collision col = sphere_collide_face(s, v, ct);
  
  if (col.collided) { return col; }
  
  col = collision_merge(col, sphere_collide_edge(s, v, ct.a, ct.b));
  col = collision_merge(col, sphere_collide_edge(s, v, ct.b, ct.c));
  col = collision_merge(col, sphere_collide_edge(s, v, ct.c, ct.a));
  col = collision_merge(col, sphere_collide_point(s, v, ct.a));
  col = collision_merge(col, sphere_collide_point(s, v, ct.b));
  col = collision_merge(col, sphere_collide_point(s, v, ct.c));
  
  return col;
  
}

static collision sphere_collide_mesh_space(sphere s, vec3 v, cmesh* cm, mat4 world, mat3 space) {
  
  if ( !cm->is_leaf ) {
  
    plane div = cm->division;
    div = plane_transform(div, world);
    div = plane_transform_space(div, space);
  
         if ( sphere_swept_inside_plane(s, v, div)  ) { return sphere_collide_mesh_space(s, v, cm->back,  world, space); }
    else if ( sphere_swept_outside_plane(s, v, div) ) { return sphere_collide_mesh_space(s, v, cm->front, world, space); }
    else {
    
      collision c0 = sphere_collide_mesh_space(s, v, cm->back,  world, space);
      collision c1 = sphere_collide_mesh_space(s, v, cm->front, world, space);
      return collision_merge(c0, c1);
      
    }
  }
  
  collision col = collision_none();
  
  for (int i = 0; i < cm->triangles_num; i++) {
    ctri ct = cm->triangles[i];
    ct = ctri_transform(ct, world);
    ct = ctri_transform_space(ct, space);
    
    /* This does not work for some reason */
    //if (sphere_swept_outside_sphere(s, v, ct.bound)) continue;
    col = collision_merge(col, sphere_collide_ctri(s, v, ct));
  }
  
  return col;

}

collision sphere_collide_mesh(sphere s, vec3 v, cmesh* m, mat4 world) {
  return sphere_collide_mesh_space(s, v, m, world, mat3_id());
}

collision ellipsoid_collide_mesh(ellipsoid e, vec3 v, cmesh* m, mat4 world) {
  
  world.xw -= e.center.x;
  world.yw -= e.center.y;
  world.zw -= e.center.z;
  
  mat3 space     = mat3_scale(vec3_div_vec3(vec3_one(), e.radiuses));
  mat3 space_inv = mat3_scale(e.radiuses);
  
  v = mat3_mul_vec3(space, v);
  
  collision c = sphere_collide_mesh_space(sphere_unit(), v, m, world, space);
  
  c.point = mat3_mul_vec3(space_inv, c.point);
  c.point = vec3_add(c.point, e.center);

  c.norm = vec3_normalize(mat3_mul_vec3(space, c.norm));
  
  return c;
  
}

void collision_response_slide(vec3* position, vec3* velocity, collision (*colfunc)(void) ) {
  
  collision col = colfunc();
  
  int count = 0;
  while (col.collided) {
    
    if (count++ == 5) {
      *velocity = vec3_zero();
      break;
    }
    
    if (vec3_length(*velocity) < 0.001) {
      *velocity = vec3_zero();
      break;
    }
    
    vec3 fwrd = vec3_mul(*velocity, col.time);
    vec3 dest = vec3_add(*position, fwrd);
    
    float len = max(vec3_length(fwrd) - 0.001, 0.0);
    vec3 move = vec3_add(*position, vec3_mul(vec3_normalize(fwrd), len));    
    vec3 proj = vec3_project(vec3_mul(*velocity, (1-col.time)), col.norm);
    
    *position = move;
    *velocity = proj;
        
    col = colfunc();
  
  }
  
  *position = vec3_add(*position, *velocity);
  

}

