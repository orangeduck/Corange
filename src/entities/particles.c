
#include "entities/particles.h"

#include "cnet.h"

#include "data/randf.h"

#include "assets/effect.h"

particles* particles_new() {
  
  particles* p = malloc(sizeof(particles));
  
  p->position = vec3_zero();
  p->rotation = mat4_id();
  p->scale = vec3_one();
  
  p->effect = asset_hndl_null();
  
  p->rate = 0;
  p->count = 0;
  p->actives = NULL;
  p->seeds = NULL;
  p->times = NULL;
  p->rotations = NULL;
  p->scales = NULL;
  p->colors = NULL;
  p->positions = NULL;
  p->velocities = NULL;
  
  p->vertex_data = NULL;

  if (net_is_client()) {
    glGenBuffers(1, &p->vertex_buff);
  }
  
  return p;
}

void particles_delete(particles* p) {
  
  free(p->actives);
  free(p->seeds);
  free(p->times);
  free(p->rotations);
  free(p->scales);
  free(p->colors);
  free(p->positions);
  free(p->velocities);
  
  free(p->vertex_data);
  
  if (net_is_client()) {
    glDeleteBuffers(1, &p->vertex_buff);
  }
  
  free(p);
  
}

void particles_set_effect(particles* p, asset_hndl e) {
  
  p->effect = e;
  p->count = ((effect*)asset_hndl_ptr(e))->count;
  p->actives = realloc(p->actives, sizeof(bool) * p->count);
  p->times = realloc(p->times, sizeof(float) * p->count);
  p->seeds = realloc(p->seeds, sizeof(float) * p->count);
  p->rotations = realloc(p->rotations, sizeof(float) * p->count);
  p->scales = realloc(p->scales, sizeof(vec3) * p->count);
  p->colors = realloc(p->colors, sizeof(vec4) * p->count);
  p->positions = realloc(p->positions, sizeof(vec3) * p->count);
  p->velocities = realloc(p->velocities, sizeof(vec3) * p->count);
  
  p->vertex_data = realloc(p->vertex_data, sizeof(float) * 18 * 6 * p->count);
 
  for (int i = 0; i < p->count; i++) {
    p->actives[i] = false;
    p->times[i] = 0;
    p->seeds[i] = randf();
    p->rotations[i] = 0;
    p->scales[i] = vec3_zero();
    p->colors[i] = vec4_zero();
    p->positions[i] = vec3_zero();
    p->velocities[i] = vec3_zero();
  }
  
}

static void add_vertex(float* data, int* index, vec3 position, vec3 normal, vec3 tangent, vec3 binormal, vec2 uvs, vec4 color) {
  
  data[*index] = position.x; (*index)++;
  data[*index] = position.y; (*index)++;
  data[*index] = position.z; (*index)++;
  
  data[*index] = normal.x; (*index)++;
  data[*index] = normal.y; (*index)++;
  data[*index] = normal.z; (*index)++;
  
  data[*index] = tangent.x; (*index)++;
  data[*index] = tangent.y; (*index)++;
  data[*index] = tangent.z; (*index)++;
  
  data[*index] = binormal.x; (*index)++;
  data[*index] = binormal.y; (*index)++;
  data[*index] = binormal.z; (*index)++;
  
  data[*index] = uvs.x;      (*index)++;
  data[*index] = uvs.y;      (*index)++;
  
  data[*index] = color.x;    (*index)++;
  data[*index] = color.y;    (*index)++;
  data[*index] = color.z;    (*index)++;
  data[*index] = color.w;    (*index)++;
  
}

static void particles_update_effect(particles* p, float timestep) {
  
  srand(time(NULL));
  float globseed = randf();
  
  effect* e = asset_hndl_ptr(p->effect);
  
  if (e->count != p->count) {
    particles_set_effect(p, p->effect);
  }
  
  p->rate = p->rate + (e->output + e->output_r * randf_n()) * timestep;
  p->rate = min(p->rate, 3);
  
  for (int i = 0; i < p->count; i++) {
    
    p->times[i] = p->times[i] + timestep;
    
    /* Spawn new particle */
    if (!p->actives[i]) {
    
      if (p->rate >= 1) {
        
        float seed = randf_seed(p->seeds[i]) + globseed;
        
        p->rate--;
        p->times[i] = 0;
        p->seeds[i] = randf_seed(seed+0);
        p->rotations[i] = randf_seed(seed+1);
        p->scales[i] = vec3_zero();
        p->positions[i] = vec3_zero();
        p->velocities[i] = vec3_zero();
        p->actives[i] = true;
      }
      
      continue;
    }
    
    /* Cleanup finished particle */
    if (p->times[i] > e->lifetime) {
      p->actives[i] = false;
      p->scales[i] = vec3_zero();
      p->positions[i] = vec3_zero();
      p->velocities[i] = vec3_zero();
      continue;
    }
    
    float seed = randf_nseed(p->seeds[i]);
    
    /* Update flight particle */
    effect_key ek = effect_get_key(e, p->times[i]);
    
    p->scales[i] = vec3_add(ek.scale, vec3_mul(ek.scale_r, randf_nseed(seed+0)));
    p->colors[i] = vec4_add(ek.color, vec4_mul(ek.color_r, randf_nseed(seed+1)));
    p->rotations[i] = p->rotations[i] + ek.rotation_r * randf_nseed(seed+2) * timestep;
    p->velocities[i] = vec3_add(p->velocities[i], vec3_mul(vec3_add(ek.force, vec3_mul(ek.force_r, randf_nseed(seed+3))), timestep));
    p->positions[i] = vec3_add(p->positions[i], vec3_mul(p->velocities[i], timestep));
    
  }

}

void particles_update(particles* p, float timestep, camera* cam) {
  
  particles_update_effect(p, timestep);
  
  int vi = 0;
  for (int i = 0; i < p->count; i++) {
    
    vec3 axisz = vec3_normalize(vec3_sub(p->position, cam->position));
    vec3 axisx = vec3_cross(axisz, vec3_up());
    vec3 axisy = vec3_cross(axisz, axisx);

    mat3 rot_camera = mat3_new(
      axisx.x, axisx.y, axisx.z,
      axisy.x, axisy.y, axisy.z,
      axisz.x, axisz.y, axisz.z);
    
    mat4 world_pos = mat4_new(
      rot_camera.xx, rot_camera.xy, rot_camera.xz, p->positions[i].x,
      rot_camera.yx, rot_camera.yy, rot_camera.yz, p->positions[i].y,
      rot_camera.zx, rot_camera.zy, rot_camera.zz, p->positions[i].z,
      0, 0, 0, 1);
    
    vec3 scale = p->actives[i] ? p->scales[i] : vec3_zero();
    
    mat3 rot_axis = mat3_rotation_z(p->rotations[i] * 2 * M_PI);
    
    vec3 vertex_0 = mat4_mul_vec3(world_pos, mat3_mul_vec3(rot_axis, vec3_new(-scale.x,  scale.y, 0)));
    vec3 vertex_1 = mat4_mul_vec3(world_pos, mat3_mul_vec3(rot_axis, vec3_new( scale.x,  scale.y, 0)));
    vec3 vertex_2 = mat4_mul_vec3(world_pos, mat3_mul_vec3(rot_axis, vec3_new( scale.x, -scale.y, 0)));
    vec3 vertex_3 = mat4_mul_vec3(world_pos, mat3_mul_vec3(rot_axis, vec3_new(-scale.x, -scale.y, 0)));
    
    vec3 normal   = mat3_mul_vec3(rot_camera, mat3_mul_vec3(rot_axis, vec3_new(0, 0, 1)));
    vec3 tangent  = mat3_mul_vec3(rot_camera, mat3_mul_vec3(rot_axis, vec3_new(1, 0, 0)));
    vec3 binormal = mat3_mul_vec3(rot_camera, mat3_mul_vec3(rot_axis, vec3_new(0, 1, 0)));
    
    add_vertex(p->vertex_data, &vi, vertex_0, normal, tangent, binormal, vec2_new(0, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_1, normal, tangent, binormal, vec2_new(1, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_2, normal, tangent, binormal, vec2_new(1, 0), p->colors[i]);
    
    add_vertex(p->vertex_data, &vi, vertex_0, normal, tangent, binormal, vec2_new(0, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_2, normal, tangent, binormal, vec2_new(1, 0), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_3, normal, tangent, binormal, vec2_new(0, 0), p->colors[i]);
    
  }
  
  if (net_is_client()) {
    glBindBuffer(GL_ARRAY_BUFFER, p->vertex_buff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * 6 * p->count, p->vertex_data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  
}
