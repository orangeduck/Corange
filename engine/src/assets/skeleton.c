#include "assets/skeleton.h"

skeleton* skeleton_new() {
  skeleton* s =  malloc(sizeof(skeleton));
  s->num_bones = 0;
  s->bones = malloc(sizeof(bone*) * s->num_bones);
  
  s->transforms = malloc(sizeof(mat4) * s->num_bones);
  s->inv_transforms = malloc(sizeof(mat4) * s->num_bones);
  
  return s;
}

skeleton* skeleton_copy(skeleton* old) {
  skeleton* new =  malloc(sizeof(skeleton));
  new->num_bones = old->num_bones;
  new->bones = malloc(sizeof(bone*) * new->num_bones);
  
  for(int i = 0; i < new->num_bones; i++) {
    new->bones[i] = bone_new(old->bones[i]->id, old->bones[i]->name);
    new->bones[i]->position = old->bones[i]->position;
    new->bones[i]->rotation = old->bones[i]->rotation;
  }
  
  for(int i = 0; i < new->num_bones; i++) {
    if (old->bones[i]->parent == NULL) {
      new->bones[i]->parent = NULL;
    } else {
      new->bones[i]->parent = skeleton_bone_id(new, old->bones[i]->parent->id);
    }
  }
  
  new->transforms = malloc(sizeof(mat4) * new->num_bones);
  new->inv_transforms = malloc(sizeof(mat4) * new->num_bones);
  
  for(int i = 0; i < new->num_bones; i++) {
    new->transforms[i] = old->transforms[i];
    new->inv_transforms[i] = old->inv_transforms[i];
  }
  
  return new;
}


void skeleton_delete(skeleton* s) {

  for(int i = 0; i < s->num_bones; i++) {
    bone_delete(s->bones[i]);
  }
  free(s->bones);
  free(s->transforms);
  free(s->inv_transforms);
  free(s);
}

void skeleton_add_bone(skeleton* s, char* name, int id, int parent_id) {
  
  bone* b = bone_new(id, name);
  b->parent = skeleton_bone_id(s, parent_id);
  
  s->num_bones++;
  s->bones = realloc(s->bones, sizeof(bone*) * s->num_bones);
  s->bones[s->num_bones-1] = b;
  
  s->transforms = realloc(s->transforms, sizeof(mat4) * s->num_bones);
  s->transforms[s->num_bones-1] = mat4_id();
  
  s->inv_transforms = realloc(s->inv_transforms, sizeof(mat4) * s->num_bones);
  s->inv_transforms[s->num_bones-1] = mat4_id();
  
}

void skeleton_print(skeleton* s) {
  for(int i = 0; i < s->num_bones; i++) {
    bone* b = s->bones[i];
    printf("Bone %i: %i %s ", i, b->id, b->name);
    vec3_print(b->position);printf(" ");
    //v4_print(b->rotation);
    if (b->parent == NULL) {
      printf(" ROOT\n");
    } else {
      printf(" %i\n", b->parent->id);
    }
  }
}

bone* skeleton_bone_id(skeleton* s, int id) {
  
  if(id == -1) {
    return NULL;
  }
  
  for(int i = 0; i < s->num_bones; i++) {
    if (s->bones[i]->id == id) {
      return s->bones[i];
    }
  }
  
  warning("unknown bone id %i", id);
  return NULL;
}

bone* skeleton_bone_name(skeleton* s, char* name) {
  
  for(int i = 0; i < s->num_bones; i++) {
    if (strcmp(s->bones[i]->name, name) == 0) {
      return s->bones[i];
    }
  }
  
  warning("unknown bone name %s", name);
  return NULL;
  
}

bone* bone_new(int id, char* name) {
  bone* b = malloc(sizeof(bone));
  b->name = malloc(strlen(name) + 1);
  strcpy(b->name, name);
  b->id = id;
  b->position = vec3_zero();
  b->rotation = mat4_id();
  b->parent = NULL;
  return b;
}

void bone_delete(bone* b) {
  free(b->name);
  free(b);
}

mat4 bone_transform(bone* b) {
  
  if (b->parent == NULL) {
    mat4 ret = mat4_id();
    mat4 trans = mat4_translation(b->position);
    mat4 rot = b->rotation;
    
    ret = mat4_mul_mat4(ret, trans);
    ret = mat4_mul_mat4(ret, rot);
    
    return ret;
  } else {
    mat4 prev = bone_transform(b->parent);
    
    mat4 ret = mat4_id();
    mat4 trans = mat4_translation(b->position);
    mat4 rot = b->rotation;
    
    ret = mat4_mul_mat4(ret, prev);
    ret = mat4_mul_mat4(ret, trans);
    ret = mat4_mul_mat4(ret, rot);
    
    return ret;
  }
}

void inverse_kinematics_solve(bone* base, bone* end, vec3 target) {
  
  if (end->parent->parent != base) {
    error("Can only solve two-joint inverse kinematics!");
  }
  
  bone* mid = end->parent;
  
  vec3 base_pos = mat4_mul_vec3(bone_transform(base), vec3_zero());
  vec3 end_pos = mat4_mul_vec3(bone_transform(end), vec3_zero());
  vec3 mid_pos = mat4_mul_vec3(bone_transform(mid), vec3_zero());
  vec3 tar_pos = target;
  
  float base_target_dist = vec3_dist(base_pos, target);
  float base_mid_dist = vec3_dist(base_pos, mid_pos);
  float mid_end_dist = vec3_dist(mid_pos, end_pos);
  
  if (base_target_dist >= base_mid_dist + mid_end_dist - 0.01) {
    vec3 target_dir = vec3_normalize(vec3_sub(target, base_pos));
    tar_pos = vec3_add(base_pos, vec3_mul(target_dir, base_mid_dist + mid_end_dist - 0.01));
  }
  
  mat4 inv_trans = mat4_inverse(bone_transform(base));
  base_pos = mat4_mul_vec3(inv_trans, base_pos);
  end_pos = mat4_mul_vec3(inv_trans, end_pos);
  mid_pos = mat4_mul_vec3(inv_trans, mid_pos);
  tar_pos = mat4_mul_vec3(inv_trans, tar_pos);
  
  vec3 base_tar = vec3_normalize(vec3_sub(tar_pos, base_pos));
  vec3 base_end = vec3_normalize(vec3_sub(end_pos, base_pos));
  vec3 rot_axis =  vec3_normalize(vec3_cross(base_tar, base_end));
  
  mat4 plane_view = mat4_view_look_at(vec3_zero(), rot_axis, vec3_new(0,1,0));
  
  /* Project onto rotation plane and convert to 2D */
  base_pos = mat4_mul_vec3(plane_view, base_pos);
  end_pos = mat4_mul_vec3(plane_view, end_pos);
  mid_pos = mat4_mul_vec3(plane_view, mid_pos);
  tar_pos = mat4_mul_vec3(plane_view, tar_pos);
  
  vec2 base_plane = vec2_new(base_pos.x, base_pos.y);
  vec2 end_plane = vec2_new(end_pos.x, end_pos.y);
  vec2 mid_plane = vec2_new(mid_pos.x, mid_pos.y);
  vec2 tar_plane = vec2_new(tar_pos.x, tar_pos.y);
  
  float l1 = vec2_dist(base_plane, mid_plane);
  float l2 = vec2_dist(mid_plane, end_plane);
  
  /* Now we can calculate rotations */
  float px = tar_plane.x;
  float py = tar_plane.y;
  
  float r2_frac = (px*px + py*py - l1*l1 - l2*l2) / (2*l1*l2);
  
  if (r2_frac < -1) {
    warning("Could not solve IK, somehow out of range!");
    return;
  }
  if (r2_frac > 1) {
    warning("Could not solve IK, somehow out of range!");
    return;
  }
  
  float r2 = acos(r2_frac);
  
  float r1_top = -(l2*sin(r2))*px + (l1 + l2*cos(r2))*py;
  float r1_bot =  (l2*sin(r2))*py + (l1 + l2*cos(r2))*px;
  float r1_frac = r1_top/r1_bot;
  float r1 = atan(r1_frac);
  
  /* Apply Rotations */
  
  mat4 mid_rotation = mat4_rotation_axis_angle(rot_axis, r2);
  mid->rotation = mid_rotation;
  
  /*
    There are quite a few different quadrants which invert it.
    Easier than working out the pattern is just trying both.
    See which one fits best.
  */
  float base_rotation = 0;
  
  mat4 base_rotation0 = mat4_mul_mat4(base->rotation, mat4_rotation_axis_angle(base_tar, base_rotation));
  mat4 base_rotation1 = mat4_rotation_axis_angle(rot_axis, r1);
  mat4 base_rotation2 = mat4_rotation_axis_angle(rot_axis, r1 + 3.14);
  
  base->rotation = mat4_mul_mat4(base_rotation0, base_rotation1);
  vec3 end_position1 = mat4_mul_vec3(bone_transform(end), vec3_zero());
  
  base->rotation = mat4_mul_mat4(base_rotation0, base_rotation2);
  vec3 end_position2 = mat4_mul_vec3(bone_transform(end), vec3_zero());
  
  if (vec3_dist_sqrd(end_position1, target) < vec3_dist_sqrd(end_position2, target)) {
    base->rotation = mat4_mul_mat4(base_rotation0, base_rotation1);
  } else {
    base->rotation = mat4_mul_mat4(base_rotation0, base_rotation2);
  }
  
}

/* TODO: These functions could be optimised to use previously calculated transforms */
void skeleton_gen_transforms(skeleton* s) {
  for(int i = 0; i < s->num_bones; i++) {
    s->transforms[i] = bone_transform(s->bones[i]);
  }
}

void skeleton_gen_inv_transforms(skeleton* s) {
  for(int i = 0; i < s->num_bones; i++) {
    s->transforms[i] = bone_transform(s->bones[i]);
    s->inv_transforms[i] = mat4_inverse(s->transforms[i]);
  }
}

static int state_load_empty = 0;
static int state_load_nodes = 1;
static int state_load_skeleton = 2;

skeleton* skl_load_file(char* filename) {
  
  int state = state_load_empty;
  
  skeleton* s =  skeleton_new();
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (state == state_load_empty) {
      
      int version;
      if (sscanf(line, "version %i", &version) > 0) {
        if (version != 1) {
          error("Can't load skl file %s. Don't know how to load version %i\n", filename, version);
        }
      }
      
      if (strstr(line, "nodes")) {
        state = state_load_nodes;
      }
      
      if (strstr(line, "skeleton")) {
        state = state_load_skeleton;
      }
    }
    
    else if (state == state_load_nodes) {
      char name[1024];
      int id, parent_id;
      if (sscanf(line, "%i %1024s %i", &id, name, &parent_id) == 3) {
        /* Bone name will probably contain quotation marks. If so remove. */
        if (name[0] == '\"') {
          for(int i = 0; i < strlen(name); i++) {
            name[i] = name[i+1];
          }
          name[strlen(name)-1] = '\0';
        }
        skeleton_add_bone(s, name, id, parent_id);
      }
      
      if (strstr(line, "end")) {
        state = state_load_empty;
      }
    }
    
    else if (state == state_load_skeleton) {
      int id;
      float x, y, z, rx, ry, rz;
      if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) == 7) {
        bone* b = skeleton_bone_id(s, id);
        /* Swap z and y */
        b->position = vec3_new(x, z, y);
        
        mat4 rotation = mat4_rotation_euler(rx, ry, rz);
        mat4 handedflip = mat4_new(1,0,0,0,
                                   0,0,1,0,
                                   0,1,0,0,
                                   0,0,0,1);
      
        rotation = mat4_mul_mat4(handedflip, rotation);
        rotation = mat4_mul_mat4(rotation, handedflip);
        rotation = mat4_transpose(rotation);
        b->rotation = rotation;
        
      }
      
      if (strstr(line, "end")) {
        state = state_load_empty;
      }
    }
  }
  
  SDL_RWclose(file);
  
  skeleton_gen_inv_transforms(s);
  
  return s;
}
