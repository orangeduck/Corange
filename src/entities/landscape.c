#include "entities/landscape.h"

#include "assets/terrain.h"
#include "assets/texture.h"

landscape_blobtree* landscape_blobtree_new(landscape* l) {

  terrain* terr = asset_hndl_ptr(&l->heightmap);
  
  vec3 scale = vec3_new(-(1.0 / terr->width) * l->size_x, l->scale, -(1.0 / terr->height) * l->size_y);
  vec3 translation = vec3_new(l->size_x / 2, 0, l->size_y / 2);
  float scale_bound = sqrt(pow(max(scale.x, scale.z), 2.0) * 2);
  
  int blob_step = 1;
  landscape_blobtree** blobs = malloc(sizeof(landscape_blobtree*) * terr->num_chunks);

  while (blob_step <= terr->num_rows) {
  
    for (int x = 0; x < terr->num_rows; x += blob_step)
    for (int y = 0; y < terr->num_cols; y += blob_step) {
      
      if (blob_step == 1) {
        
        terrain_chunk* tc = terr->chunks[x + y * terr->num_rows];
        
        sphere bbound = tc->bound;
        bbound.center = vec3_mul_vec3(bbound.center, scale);
        bbound.center = vec3_add(bbound.center, translation);
        bbound.radius = bbound.radius * scale_bound;
        
        blobs[x + y * terr->num_rows] = malloc(sizeof(landscape_blobtree));
        blobs[x + y * terr->num_rows]->bound = bbound;
        blobs[x + y * terr->num_rows]->is_leaf = true;
        blobs[x + y * terr->num_rows]->chunk_index = x + y * terr->num_rows;
        blobs[x + y * terr->num_rows]->child0 = NULL;
        blobs[x + y * terr->num_rows]->child1 = NULL;
        blobs[x + y * terr->num_rows]->child2 = NULL;
        blobs[x + y * terr->num_rows]->child3 = NULL;
        
      } else {
        
        int i = blob_step/2;
        landscape_blobtree* child0 = blobs[(x+0) + (y+0) * terr->num_rows];
        landscape_blobtree* child1 = blobs[(x+i) + (y+0) * terr->num_rows];
        landscape_blobtree* child2 = blobs[(x+0) + (y+i) * terr->num_rows];
        landscape_blobtree* child3 = blobs[(x+i) + (y+i) * terr->num_rows];
        
        sphere bounds[4] = { child0->bound, child1->bound, child2->bound, child3->bound };
        
        blobs[x + y * terr->num_rows] = malloc(sizeof(landscape_blobtree));
        blobs[x + y * terr->num_rows]->bound = sphere_merge_many(bounds, 4);
        blobs[x + y * terr->num_rows]->is_leaf = false;
        blobs[x + y * terr->num_rows]->chunk_index = -1;
        blobs[x + y * terr->num_rows]->child0 = child0;
        blobs[x + y * terr->num_rows]->child1 = child1;
        blobs[x + y * terr->num_rows]->child2 = child2;
        blobs[x + y * terr->num_rows]->child3 = child3;
        
      }
      
    }
    
    blob_step *= 2;
    
  }
  
  landscape_blobtree* root = blobs[0];
  free(blobs);
  
  return root;
  
}

void landscape_blobtree_delete(landscape_blobtree* lbt) {
  
  if (!lbt->is_leaf) {
    landscape_blobtree_delete(lbt->child0);
    landscape_blobtree_delete(lbt->child1);
    landscape_blobtree_delete(lbt->child2);
    landscape_blobtree_delete(lbt->child3);
  }
  
  free(lbt);
  
}

void landscape_blobtree_generate(landscape* l) {
  
  if (l->blobtree != NULL) {
    landscape_blobtree_delete(l->blobtree);
  }
  
  l->blobtree = landscape_blobtree_new(l);
  
}

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  
  l->heightmap = asset_hndl_null();
  l->attribmap = asset_hndl_null();
  
  l->attribimage = NULL;
  
  l->scale = 0.25;
  l->size_x = 128;
  l->size_y = 128;
  l->blobtree = NULL;
  
  l->ground0 = asset_hndl_null();
  l->ground1 = asset_hndl_null();
  l->ground2 = asset_hndl_null();
  l->ground3 = asset_hndl_null();
  
  l->ground0_nm = asset_hndl_null();
  l->ground1_nm = asset_hndl_null();
  l->ground2_nm = asset_hndl_null();
  l->ground3_nm = asset_hndl_null();
  
  return l;
  
}

void landscape_delete(landscape* l) {
  
  if (l->attribimage != NULL) { image_delete(l->attribimage); }
  if (l->blobtree != NULL) { landscape_blobtree_delete(l->blobtree); }
  
  free(l);
}

mat4  landscape_world(landscape* l) {
  
  terrain* terr = asset_hndl_ptr(&l->heightmap);
  
  vec3 scale = vec3_new(-(1.0 / terr->width) * l->size_x, l->scale, -(1.0 / terr->height) * l->size_y);
  vec3 translation = vec3_new(l->size_x / 2, 0, l->size_y / 2);
  
  return mat4_world(translation, scale, quat_id());
  
}

float landscape_height(landscape* l, vec2 pos) {
  
  terrain* t = asset_hndl_ptr(&l->heightmap);
  
  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  return l->scale * terrain_height(t, pos);
  
}

vec3 landscape_normal(landscape* l, vec2 pos) {
  
  terrain* t = asset_hndl_ptr(&l->heightmap);

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  vec3 norm = terrain_normal(t, pos);
  
  vec3 scale = vec3_new(-(1.0 / t->width) * l->size_x, l->scale, -(1.0 / t->height) * l->size_y);
  vec3 translation = vec3_new(l->size_x / 2, 0, l->size_y / 2);
  
  mat4 world = mat4_world(translation, scale, quat_id());
  
  return mat3_mul_vec3(mat4_to_mat3(world), norm);

}

mat3 landscape_axis(landscape* l, vec2 pos) {

  terrain* t = asset_hndl_ptr(&l->heightmap);

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  mat3 axis = terrain_axis(t, pos);
  
  return axis;

}

void landscape_paint_height(landscape* l, vec2 pos, float radius, float value, float opacity) {

  terrain* t = asset_hndl_ptr(&l->heightmap);

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  int base_x = roundf(pos.x);
  int base_y = roundf(pos.y);
  
  for (int x = base_x - radius - 1; x < base_x + radius + 1; x++)
  for (int y = base_y - radius - 1; y < base_y + radius + 1; y++) {
    
    if (x < 0) continue;
    if (y < 0) continue;
    if (x >= t->width) continue;
    if (y >= t->height) continue;
    
    float dist = saturate(1 - vec2_dist(pos, vec2_new(x, y)) / radius);
    
    t->heightmap[x + y * t->width] = max(t->heightmap[x + y * t->width] + value * dist * opacity, 0);
  }
  
  int chunk_x = base_x / t->chunk_width;
  int chunk_y = base_y / t->chunk_height;
  
  for(int x = chunk_x - 1; x < chunk_x + 2; x++)
  for(int y = chunk_y - 1; y < chunk_y + 2; y++) {
    int chunk = clamp(x + y * t->num_rows, 0, t->num_chunks-1);
    terrain_reload_chunk(t, chunk);
  }
  
}

void landscape_chunks(landscape* l, vec2 pos, struct terrain_chunk** chunks_out) {
  
  terrain* t = asset_hndl_ptr(&l->heightmap);

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  int base_x = roundf(pos.x);
  int base_y = roundf(pos.y);
  
  int chunk_x = base_x / t->chunk_width;
  int chunk_y = base_y / t->chunk_height;
  
  int index = 0;
  for(int x = chunk_x - 1; x < chunk_x + 2; x++)
  for(int y = chunk_y - 1; y < chunk_y + 2; y++) {
    int chunk = clamp(x + y * t->num_rows, 0, t->num_chunks-1);
    chunks_out[index] = t->chunks[chunk]; index++;
  }
  
}

void landscape_paint_color(landscape* l, vec2 pos, float radius, int type, float opacity) {
  
  if (l->attribimage == NULL) {
    l->attribimage = texture_get_image(asset_hndl_ptr(&l->attribmap));
  }

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * l->attribimage->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * l->attribimage->height;
  
  int base_x = roundf(pos.x);
  int base_y = roundf(pos.y);
  
  for (int x = base_x - radius - 1; x < base_x + radius + 1; x++)
  for (int y = base_y - radius - 1; y < base_y + radius + 1; y++) {
    
    if (x < 0) continue;
    if (y < 0) continue;
    if (x >= l->attribimage->width) continue;
    if (y >= l->attribimage->height) continue;
    
    float dist = saturate(1 - vec2_dist(pos, vec2_new(x, y)) / radius) * opacity;
    
    vec4 pix = image_get_pixel(l->attribimage, x, y);
    
    if (type == 0) { pix.x += dist; }
    if (type == 1) { pix.y += dist; }
    if (type == 2) { pix.z += dist; }
    if (type == 3) { pix.w += dist; }
    
    pix = vec4_normalize(pix);
    
    image_set_pixel(l->attribimage, x, y, pix);
  }
  
  texture_set_image(asset_hndl_ptr(&l->attribmap), l->attribimage);
  
}
