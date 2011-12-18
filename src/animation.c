#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL/SDL_rwops.h"
#include "SDL/SDL_local.h"

#include "error.h"

#include "animation.h"

animation* animation_new() {
  
  animation* a = malloc(sizeof(animation));
  
  a->start_time = 1;
  a->end_time = 1;
  
  a->num_frames = 0;
  a->frame_times = malloc(sizeof(float) * 0);
  a->frames = malloc(sizeof(skeleton*) * 0);
  
  return a;
}

animation* animation_identity(skeleton* s);

void animation_delete(animation* a) {
  int i;
  for(i = 0; i < a->num_frames; i++) {
    skeleton_delete(a->frames[i]);
  }
  
  free(a->frame_times);
  free(a);
}

skeleton* animation_new_frame(animation* a, float frametime, skeleton* base) {
  
  skeleton* frame;
  
  if(a->num_frames == 0) {
    frame = skeleton_copy(base);
  } else {
    frame = skeleton_copy(a->frames[a->num_frames-1]);
  }
  
  a->num_frames++;
  a->frame_times = realloc(a->frame_times, sizeof(float) * a->num_frames);
  a->frames = realloc(a->frames, sizeof(skeleton*) * a->num_frames);
  a->frame_times[a->num_frames-1] = frametime;
  a->frames[a->num_frames-1] = frame;
    
  return frame;
  
}

static int state_load_empty = 0;
static int state_load_skeleton = 1;
static int state_load_nodes = 2;

animation* ani_load_file(char* filename) {
  
  int state = state_load_empty;
  
  animation* a =  animation_new();
  skeleton* base = skeleton_new();
  skeleton* frame = NULL;
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    /* Process line */
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
        skeleton_add_bone(base, name, id, parent_id);
      }
      
      if (strstr(line, "end")) {
        state = state_load_empty;
      }
    }
    
    else if (state == state_load_skeleton) {
    
      float time;
      if (sscanf(line, "time %f", &time) == 1) {
        frame = animation_new_frame(a, time, base);
        a->end_time = max(a->end_time, time);
        if(time != 0)
        a->start_time = min(a->start_time, time);
      }
    
      int id;
      float x, y, z, rx, ry, rz;
      if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) > 0) {
        bone* b = skeleton_bone_id(frame, id);
        /* Swap z and y */
        b->position = v3(x, z, y);
        
        matrix_4x4 rotation = m44_rotation_euler(rx, ry, rz);
        matrix_4x4 handedflip = m44(1,0,0,0,
                                    0,0,1,0,
                                    0,1,0,0,
                                    0,0,0,1);
        
        rotation = m44_mul_m44(handedflip, rotation);
        rotation = m44_mul_m44(rotation, handedflip);
        rotation = m44_transpose(rotation);
        b->rotation = rotation;
        
      }
      
      if (strstr(line, "end")) {
        state = state_load_empty;
      }
    }
  }
  
  SDL_RWclose(file);
  
  return a;
}
