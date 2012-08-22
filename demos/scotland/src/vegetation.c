#include <time.h>
#include <math.h>

#include "vegetation.h"

static static_object** blocks;

static int WIDTH = 1024;
static int HEIGHT = 1024;
static int BLOCK_SIZE = 32;
static int X_NUM = 32; /* (1024 / 32) */
static int Y_NUM = 32; /* (1024 / 32) */

void vegetation_init() {
  
  blocks = malloc(X_NUM * Y_NUM * sizeof(static_object*));
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    
    static_object* block = entity_new(blockname, static_object);
    block->renderable = asset_hndl_null();
    
    blocks[x + y*X_NUM] = block;
  }

}

void vegetation_finish() {
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    
    static_object* block = entity_get(blockname);
    //renderable_delete(block->renderable);
    
    entity_delete(blockname);
  }
  
  free(blocks);
  
}

void vegetation_update() {

}

void vegetation_render() {
  
  camera* cam = entity_get("camera");
  
  vec2 pos = vec2_div(vec2_new(cam->position.x, cam->position.z), BLOCK_SIZE);
  pos = vec2_clamp(pos, 1, 1022);
  
  char blockname0[512]; char blockname1[512]; char blockname2[512];
  char blockname3[512]; char blockname4[512]; char blockname5[512];
  char blockname6[512]; char blockname7[512]; char blockname8[512];
  
  sprintf(blockname0, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y-1);
  sprintf(blockname1, "vegblock_%i_%i", (int)pos.x  , (int)pos.y-1);
  sprintf(blockname2, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y-1);
  sprintf(blockname3, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y  );
  sprintf(blockname4, "vegblock_%i_%i", (int)pos.x  , (int)pos.y  );
  sprintf(blockname5, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y  );
  sprintf(blockname6, "vegblock_%i_%i", (int)pos.x-1, (int)pos.y+1);
  sprintf(blockname7, "vegblock_%i_%i", (int)pos.x  , (int)pos.y+1);
  sprintf(blockname8, "vegblock_%i_%i", (int)pos.x+1, (int)pos.y+1);
  
  static_object* block0 = entity_get(blockname0);
  static_object* block1 = entity_get(blockname1);
  static_object* block2 = entity_get(blockname2);
  static_object* block3 = entity_get(blockname3);
  static_object* block4 = entity_get(blockname4);
  static_object* block5 = entity_get(blockname5);
  static_object* block6 = entity_get(blockname6);
  static_object* block7 = entity_get(blockname7);
  static_object* block8 = entity_get(blockname8);
  
  /*
  forward_renderer_render_static(block0);
  forward_renderer_render_static(block1);
  forward_renderer_render_static(block2);
  forward_renderer_render_static(block3);
  forward_renderer_render_static(block4);
  forward_renderer_render_static(block5);
  forward_renderer_render_static(block6);
  forward_renderer_render_static(block7);
  forward_renderer_render_static(block8);
  */
  
}

static float random_in_range(float bottom, float top) {
  float difference = top - bottom;
  float random = (float)rand() / RAND_MAX;
  return bottom + random * difference;
}

static void block_part_add_template(model* blockpart, model* template, int index) {
  
  for(int i = 0; i < blockpart->num_meshes; i++) {
    mesh* tm = template->meshes[i];
    mesh* bm = blockpart->meshes[i];
  
    int vert_off = index * tm->num_verts;
    int tri_off = index * tm->num_triangles * 3;
    
    for(int j = 0; j < tm->num_verts; j++) {
      bm->verticies[vert_off + j] = tm->verticies[j];
    }
    
    for(int j = 0; j < tm->num_triangles * 3; j++) {
      bm->triangles[tri_off + j] = tri_off + tm->triangles[j];
    }
    
  }
  
}

static void paint_vegetation(model* m) {
  
  for(int i = 0; i < m->num_meshes; i++) {
  
    mesh* me = m->meshes[i];
    for(int j = 0; j < me->num_verts; j++) {
      if (me->verticies[j].position.y < 0) {
        me->verticies[j].color = vec4_black();
      } else {
        me->verticies[j].color = vec4_white();
      }
    }
    
  }
  
}

void vegetation_add_type(terrain* t, renderable* r, float density) {
  
  model* template = renderable_to_model(r);
  model_scale(template, 1.5);
  
  paint_vegetation(template);
  
  srand(time(NULL));
  
  for(int x = 0; x < X_NUM; x++)
  for(int y = 0; y < Y_NUM; y++) {
    
    char blockname[512];
    sprintf(blockname, "vegblock_%i_%i", x, y);
    static_object* block = entity_get(blockname);
    
    int count = density * BLOCK_SIZE * BLOCK_SIZE;
    
    model* block_part = model_new();
    block_part->num_meshes = template->num_meshes;
    block_part->meshes = realloc(block_part->meshes, sizeof(mesh*) * template->num_meshes);
    
    for(int i = 0; i < template->num_meshes; i++) {
      
      block_part->meshes[i] = mesh_new();
      
      mesh* bm = block_part->meshes[i];
      mesh* tm = template->meshes[i];
      
      bm->num_verts = tm->num_verts * count;
      bm->num_triangles = tm->num_triangles * count;
      
      bm->verticies = realloc(bm->verticies, sizeof(vertex) * bm->num_verts);
      bm->triangles = realloc(bm->triangles, sizeof(int) * bm->num_triangles * 3);
    }
    
    for(int i = 0; i < count; i++) {
      
      float pos_x = random_in_range(x * BLOCK_SIZE, (x+1) * BLOCK_SIZE);
      float pos_z = random_in_range(y * BLOCK_SIZE, (y+1) * BLOCK_SIZE);
      float pos_y = terrain_height(t, vec2_new(pos_x, pos_z));
      
      float scale = random_in_range(0.8, 1.5);
      float rotation = random_in_range(0, 6.282);
      
      mat4 world = mat4_world( vec3_new(pos_x, pos_y, pos_z), vec3_new(scale, scale, scale), quaternion_yaw(rotation) );
      mat4 inv_world = mat4_inverse(world);
      
      model_transform(template, world);
      block_part_add_template(block_part, template, i);
      model_transform(template, inv_world);
    }
    
    //renderable_add_model(block->renderable, block_part);
    //renderable_set_material(block->renderable, asset_get("./resources/vegetation/grass.mat"));
    model_delete(block_part);
  }
  
  model_delete(template);

}
