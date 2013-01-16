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
  
  collision col = {
    false, FLT_MAX,
    vec3_zero(),
  };
  
  return col;
  
}

collision collision_new(float time, vec3 point) {
  
  collision col = {
    true, time,
    point,
  };
  
  return col;
}

collision collision_merge(collision c0, collision c1) {
  
  collision c;
  c.collided = c0.collided || c1.collided;
  c.time = min(c0.time, c1.time);
  c.point  = c0.time < c1.time ? c0.point  : c1.point;
  
  return c;
}

static bool point_in_triangle(vec3 point, vec3 v0, vec3 v1, vec3 v2) {
  
  vec3 d0 = vec3_sub(v2, v0);
  vec3 d1 = vec3_sub(v1, v0);
  vec3 d2 = vec3_sub(point, v0);

  float dot00 = vec3_dot(d0, d0);
  float dot01 = vec3_dot(d0, d1);
  float dot02 = vec3_dot(d0, d2);
  float dot11 = vec3_dot(d1, d1);
  float dot12 = vec3_dot(d1, d2);

  float inv_dom = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * inv_dom;
  float v = (dot00 * dot12 - dot01 * dot02) * inv_dom;

  return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

collision sphere_collide_face(sphere s, vec3 v, ctri ct) {
  
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
  
  if (!point_in_triangle(spoint, ct.a, ct.b, ct.c)) {
    return collision_none();
  } else {
    return collision_new(t, spoint);
  }
  
}

collision sphere_collide_edge(sphere s, vec3 v, vec3 e0, vec3 e1) {

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
    return collision_new(t, spoint);
  }
  
}

collision sphere_collide_point(sphere s, vec3 v, vec3 p) {

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
  
  return collision_new(t, p);
  
}

collision sphere_collide_ctri(sphere s, vec3 v, ctri ct) {
  
  if (sphere_swept_outside_sphere(s, v, ct.bound)) {
    return collision_none();
  }
  
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
  
  vec3 sv = mat3_mul_vec3(space, v);
  
  if ( !cm->is_leaf ) {
    
    plane div = cm->division;
    div = plane_transform(div, world);
    div = plane_transform_space(div, space);
  
         if ( sphere_swept_inside_plane(s, sv, div)  ) { return sphere_collide_mesh_space(s, v, cm->back,  world, space); }
    else if ( sphere_swept_outside_plane(s, sv, div) ) { return sphere_collide_mesh_space(s, v, cm->front, world, space); }
    else if ( sphere_swept_intersects_plane(s, sv, div) ) {
    
      collision c0 = sphere_collide_mesh_space(s, v, cm->back,  world, space);
      collision c1 = sphere_collide_mesh_space(s, v, cm->front, world, space);
      return collision_merge(c0, c1);
      
    }
  }
  
  sphere bound = cm->bound;
  bound = sphere_transform(bound, world);
  bound = sphere_transform_space(bound, space);
  
  if (sphere_swept_outside_sphere(s, sv, bound)) {
    return collision_none();
  }
  
  collision col = collision_none();
  mat3 ispace = mat3_inverse(space);
  
  for (int i = 0; i < cm->triangles_num; i++) {
    ctri ct = cm->triangles[i];
    ct = ctri_transform(ct, world);
    ct = ctri_transform_space(ct, space);
    
    collision c = sphere_collide_ctri(s, sv, ct);
    c.point = mat3_mul_vec3(ispace, c.point);
    
    col = collision_merge(col, c);
  }
  
  return col;
  
}

collision sphere_collide_mesh(sphere s, vec3 v, cmesh* cm, mat4 w) {
  return sphere_collide_mesh_space(s, v, cm, w, mat3_id());
}

collision ellipsoid_collide_mesh(ellipsoid e, vec3 v, cmesh* m, mat4 mworld) {
  
  mworld = mat4_mul_mat4(mat4_translation(vec3_neg(e.center)), mworld);

  collision c = sphere_collide_mesh_space(sphere_unit(), v, m, mworld, ellipsoid_space(e));
  
  c.point = vec3_add(c.point, e.center);
  
  return c;
  
}
