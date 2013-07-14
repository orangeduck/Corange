#include "assets/animation.h"

animation* animation_new() {
  
  animation* a = malloc(sizeof(animation));
  
  a->frame_count = 0;
  a->frame_time = 1.0/30.0;
  a->frames = NULL;
  
  return a;
}

void animation_delete(animation* a) {
  
  for(int i = 0; i < a->frame_count; i++) {
    frame_delete(a->frames[i]);
  }
  
  free(a->frames);
  free(a);
}

frame* animation_add_frame(animation* a, frame* base) {
  
  frame* f = frame_copy(base);
  
  a->frame_count++;
  a->frames = realloc(a->frames, sizeof(frame*) * a->frame_count);
  a->frames[a->frame_count-1] = f;
  
  return f;
  
}

float animation_duration(animation* a) {
  return a->frame_count * a->frame_time;
}

frame* animation_sample(animation* a, float time) {
  
  if (a->frame_count == 0) {
    error("Animation has no frames!");
    return NULL;
  }
  
  frame* f = frame_copy(a->frames[0]);
  animation_sample_to(a, time, f);
  return f;
  
}

frame* animation_frame(animation* a, int i) {
  i = i < 0 ? 0 : i;
  i = i > (a->frame_count-1) ? (a->frame_count-1) : i;
  return a->frames[i];
}

void animation_sample_to(animation* a, float time, frame* out) {
  
  if (a->frame_count == 0) {
    error("Animation has no frames!");
    return;
  }
  
  if (a->frame_count == 1) {
    frame_copy_to(a->frames[0], out);
    return;
  }
  
  time = fmod(time, a->frame_time * (a->frame_count-1));
  
  frame* frame0 = animation_frame(a, (time / a->frame_time) + 0);
  frame* frame1 = animation_frame(a, (time / a->frame_time) + 1);
  float amount  = fmod(time / a->frame_time, 1.0);
  
  frame_interpolate_to(frame0, frame1, amount, out);

}

enum {
  STATE_LOAD_EMPTY    = 0,
  STATE_LOAD_SKELETON = 1,
  STATE_LOAD_NODES    = 2,
};

animation* ani_load_file(char* filename) {
  
  int state = STATE_LOAD_EMPTY;
  
  animation* a =  animation_new();
  skeleton* base = skeleton_new();
  frame* f = NULL;
  
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
          error("Can't load ani file '%s'. Don't know how to load version %i\n", filename, version);
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
        skeleton_joint_add(base, name, parent);
      }
      
      if (strstr(line, "end")) {
        state = STATE_LOAD_EMPTY;
      }
    }
    
    else if (state == STATE_LOAD_SKELETON) {
    
      float time;
      if (sscanf(line, "time %f", &time) == 1) {
        f = animation_add_frame(a, base->rest_pose);
      }
    
      int id;
      float x, y, z, rx, ry, rz;
      if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) > 0) {
        
        f->joint_positions[id] = vec3_new(x, z, y);
        
        mat4 rotation = mat4_rotation_euler(rx, ry, rz);
        mat4 handedflip = mat4_new(1,0,0,0,
                                   0,0,1,0,
                                   0,1,0,0,
                                   0,0,0,1);
      
        rotation = mat4_mul_mat4(handedflip, rotation);
        rotation = mat4_mul_mat4(rotation, handedflip);
        rotation = mat4_transpose(rotation);
        
        f->joint_rotations[id] = mat4_to_quat(rotation);
        
      }
      
      if (strstr(line, "end")) {
        state = STATE_LOAD_EMPTY;
      }
      
    }
  }
  
  SDL_RWclose(file);
  
  skeleton_delete(base);
  
  return a;
}
