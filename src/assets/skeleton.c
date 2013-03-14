#include "assets/skeleton.h"

frame* frame_new() {
  frame* f = malloc(sizeof(frame));
  f->joint_count = 0;
  f->joint_parents = NULL;
  f->joint_positions = NULL;
  f->joint_rotations = NULL;
  f->transforms = NULL;
  f->transforms_inv = NULL;
  return f;
}

frame* frame_copy(frame* f) {
  
  frame* fn = frame_new();
  
  for (int i = 0; i < f->joint_count; i++) {
    frame_joint_add(fn, f->joint_parents[i], f->joint_positions[i], f->joint_rotations[i]);
  }
  
  return fn;

}

frame* frame_interpolate(frame* f0, frame* f1, float amount) {

  frame* fn = frame_copy(f0);
  frame_interpolate_to(f0, f1, amount, fn);
  return fn;

}

void frame_interpolate_to(frame* f0, frame* f1, float amount, frame* out) {

  for (int i = 0; i < out->joint_count; i++) {
    out->joint_positions[i] = vec3_lerp(f0->joint_positions[i], f1->joint_positions[i], amount);
    out->joint_rotations[i] = quat_slerp(f0->joint_rotations[i], f1->joint_rotations[i], amount);
  }

}

void frame_delete(frame* f) {
  
  free(f->joint_parents);
  free(f->joint_positions);
  free(f->joint_rotations);
  free(f->transforms);
  free(f->transforms_inv);
  free(f);
  
}

mat4 frame_joint_transform(frame* f, int i) {
  
  if (f->joint_parents[i] == -1) {
  
    mat4 ret = mat4_id();
    mat4 pos = mat4_translation(f->joint_positions[i]);
    mat4 rot = mat4_rotation_quat(f->joint_rotations[i]);
    
    ret = mat4_mul_mat4(ret, pos);
    ret = mat4_mul_mat4(ret, rot);
    
    return ret;
    
  } else {
  
    mat4 prev = frame_joint_transform(f, f->joint_parents[i]);
    
    mat4 ret = mat4_id();
    mat4 pos = mat4_translation(f->joint_positions[i]);
    mat4 rot = mat4_rotation_quat(f->joint_rotations[i]);
    
    ret = mat4_mul_mat4(ret, prev);
    ret = mat4_mul_mat4(ret, pos);
    ret = mat4_mul_mat4(ret, rot);
    
    return ret;
  }
  
}

void frame_joint_add(frame* f, int parent, vec3 position, quat rotation) {
  
  f->joint_count++;
  f->joint_parents = realloc(f->joint_parents, sizeof(int) * f->joint_count);
  f->joint_positions = realloc(f->joint_positions, sizeof(vec3) * f->joint_count);
  f->joint_rotations = realloc(f->joint_rotations, sizeof(quat) * f->joint_count);
  f->transforms = realloc(f->transforms, sizeof(mat4) * f->joint_count);
  f->transforms_inv = realloc(f->transforms_inv, sizeof(mat4) * f->joint_count);
  
  f->joint_parents[f->joint_count-1] = parent;
  f->joint_positions[f->joint_count-1] = position;
  f->joint_rotations[f->joint_count-1] = rotation;
  f->transforms[f->joint_count-1] = mat4_id();
  f->transforms_inv[f->joint_count-1] = mat4_id();
  
}

void frame_gen_transforms(frame* f) {
  
  for (int i = 0; i < f->joint_count; i++) {
    f->transforms[i] = frame_joint_transform(f, i);
  }
  
}

void frame_gen_inv_transforms(frame* f) {

  for (int i = 0; i < f->joint_count; i++) {
    f->transforms[i] = frame_joint_transform(f, i);
    f->transforms_inv[i] = mat4_inverse(f->transforms[i]);
  }

}

skeleton* skeleton_new() {
  
  skeleton* s = malloc(sizeof(skeleton));
  s->joint_count = 0;
  s->joint_names = NULL;
  s->rest_pose = frame_new();
  return s;
  
}

void skeleton_delete(skeleton* s) {
  
  for (int i = 0; i < s->joint_count; i++) {
    free(s->joint_names[i]);
  }
  free(s->joint_names);
  
  frame_delete(s->rest_pose);
  free(s);
  
}

void skeleton_joint_add(skeleton* s, char* name, int parent) {
  
  s->joint_count++;
  s->joint_names = realloc(s->joint_names, sizeof(char*) * s->joint_count);
  s->joint_names[s->joint_count-1] = malloc(strlen(name)+1);
  strcpy(s->joint_names[s->joint_count-1], name);
  
  frame_joint_add(s->rest_pose, parent, vec3_zero(), quat_id());
  
}

enum {
  STATE_LOAD_EMPTY    = 0,
  STATE_LOAD_SKELETON = 1,
  STATE_LOAD_NODES    = 2,
};

skeleton* skl_load_file(char* filename) {
  
  int state = STATE_LOAD_EMPTY;
  
  skeleton* s =  skeleton_new();
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (state == STATE_LOAD_EMPTY) {
      
      int version;
      if (sscanf(line, "version %i", &version) > 0) {
        if (version != 1) {
          error("Can't load skl file %s. Don't know how to load version %i\n", filename, version);
        }
      }
      
      if (strstr(line, "nodes")) {
        state = STATE_LOAD_NODES;
      }
      
      if (strstr(line, "skeleton")) {
        state = STATE_LOAD_SKELETON;
      }
    }
    
    else if (state == STATE_LOAD_NODES) {
      char name[1024];
      int id, parent;
      if (sscanf(line, "%i \"%[^\"]\" %i", &id, name, &parent) == 3) {
        skeleton_joint_add(s, name, parent);
      }
      
      if (strstr(line, "end")) {
        state = STATE_LOAD_EMPTY;
      }
    }
    
    else if (state == STATE_LOAD_SKELETON) {
      int id;
      float x, y, z, rx, ry, rz;
      if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) == 7) {
        
        /* Swap z and y */
        s->rest_pose->joint_positions[id] = vec3_new(x, z, y);
        
        mat4 rotation = mat4_rotation_euler(rx, ry, rz);
        mat4 handedflip = mat4_new(1,0,0,0,
                                   0,0,1,0,
                                   0,1,0,0,
                                   0,0,0,1);
      
        rotation = mat4_mul_mat4(handedflip, rotation);
        rotation = mat4_mul_mat4(rotation, handedflip);
        rotation = mat4_transpose(rotation);
        
        s->rest_pose->joint_rotations[id] = mat4_to_quat(rotation);
        
      }
      
      if (strstr(line, "end")) {
        state = STATE_LOAD_EMPTY;
      }
    }
  }
  
  SDL_RWclose(file);
  
  frame_gen_transforms(s->rest_pose);
  frame_gen_inv_transforms(s->rest_pose);
  
  return s;
}



















// static void inverse_kinematics_solve(bone* base, bone* end, vec3 target) {
  
  // if (end->parent->parent != base) {
    // error("Can only solve two-joint inverse kinematics!");
  // }
  
  // bone* mid = end->parent;
  
  // vec3 base_pos = mat4_mul_vec3(bone_transform(base), vec3_zero());
  // vec3 end_pos = mat4_mul_vec3(bone_transform(end), vec3_zero());
  // vec3 mid_pos = mat4_mul_vec3(bone_transform(mid), vec3_zero());
  // vec3 tar_pos = target;
  
  // float base_target_dist = vec3_dist(base_pos, target);
  // float base_mid_dist = vec3_dist(base_pos, mid_pos);
  // float mid_end_dist = vec3_dist(mid_pos, end_pos);
  
  // if (base_target_dist >= base_mid_dist + mid_end_dist - 0.01) {
    // vec3 target_dir = vec3_normalize(vec3_sub(target, base_pos));
    // tar_pos = vec3_add(base_pos, vec3_mul(target_dir, base_mid_dist + mid_end_dist - 0.01));
  // }
  
  // mat4 inv_trans = mat4_inverse(bone_transform(base));
  // base_pos = mat4_mul_vec3(inv_trans, base_pos);
  // end_pos = mat4_mul_vec3(inv_trans, end_pos);
  // mid_pos = mat4_mul_vec3(inv_trans, mid_pos);
  // tar_pos = mat4_mul_vec3(inv_trans, tar_pos);
  
  // vec3 base_tar = vec3_normalize(vec3_sub(tar_pos, base_pos));
  // vec3 base_end = vec3_normalize(vec3_sub(end_pos, base_pos));
  // vec3 rot_axis =  vec3_normalize(vec3_cross(base_tar, base_end));
  
  // mat4 plane_view = mat4_view_look_at(vec3_zero(), rot_axis, vec3_new(0,1,0));
  
  // /* Project onto rotation plane and convert to 2D */
  // base_pos = mat4_mul_vec3(plane_view, base_pos);
  // end_pos = mat4_mul_vec3(plane_view, end_pos);
  // mid_pos = mat4_mul_vec3(plane_view, mid_pos);
  // tar_pos = mat4_mul_vec3(plane_view, tar_pos);
  
  // vec2 base_plane = vec2_new(base_pos.x, base_pos.y);
  // vec2 end_plane = vec2_new(end_pos.x, end_pos.y);
  // vec2 mid_plane = vec2_new(mid_pos.x, mid_pos.y);
  // vec2 tar_plane = vec2_new(tar_pos.x, tar_pos.y);
  
  // float l1 = vec2_dist(base_plane, mid_plane);
  // float l2 = vec2_dist(mid_plane, end_plane);
  
  // /* Now we can calculate rotations */
  // float px = tar_plane.x;
  // float py = tar_plane.y;
  
  // float r2_frac = (px*px + py*py - l1*l1 - l2*l2) / (2*l1*l2);
  
  // if (r2_frac < -1) {
    // warning("Could not solve IK, somehow out of range!");
    // return;
  // }
  // if (r2_frac > 1) {
    // warning("Could not solve IK, somehow out of range!");
    // return;
  // }
  
  // float r2 = acos(r2_frac);
  
  // float r1_top = -(l2*sin(r2))*px + (l1 + l2*cos(r2))*py;
  // float r1_bot =  (l2*sin(r2))*py + (l1 + l2*cos(r2))*px;
  // float r1_frac = r1_top/r1_bot;
  // float r1 = atan(r1_frac);
  
  // /* Apply Rotations */
  
  // mat4 mid_rotation = mat4_rotation_axis_angle(rot_axis, r2);
  // mid->rotation = mid_rotation;
  
  // /*
    // There are quite a few different quadrants which invert it.
    // Easier than working out the pattern is just trying both.
    // See which one fits best.
  // */
  // float base_rotation = 0;
  
  // mat4 base_rotation0 = mat4_mul_mat4(base->rotation, mat4_rotation_axis_angle(base_tar, base_rotation));
  // mat4 base_rotation1 = mat4_rotation_axis_angle(rot_axis, r1);
  // mat4 base_rotation2 = mat4_rotation_axis_angle(rot_axis, r1 + 3.14);
  
  // base->rotation = mat4_mul_mat4(base_rotation0, base_rotation1);
  // vec3 end_position1 = mat4_mul_vec3(bone_transform(end), vec3_zero());
  
  // base->rotation = mat4_mul_mat4(base_rotation0, base_rotation2);
  // vec3 end_position2 = mat4_mul_vec3(bone_transform(end), vec3_zero());
  
  // if (vec3_dist_sqrd(end_position1, target) < vec3_dist_sqrd(end_position2, target)) {
    // base->rotation = mat4_mul_mat4(base_rotation0, base_rotation1);
  // } else {
    // base->rotation = mat4_mul_mat4(base_rotation0, base_rotation2);
  // }
  
// }

