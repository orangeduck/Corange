#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

#include "asset_manager.h"

#include "animation.h"

frame* frame_new(float time) {
  frame* f = malloc(sizeof(frame));
  f->time = time;
  f->num_bone_frames = 0;
  f->bone_frames = malloc(sizeof(bone_frame) * 0);
  return f;
}

void frame_delete(frame* f) {
  free(f->bone_frames);
  free(f);
}

void frame_add_bone_frame(frame* f, int id, vector3 pos, vector4 rot) {
  f->num_bone_frames++;
  f->bone_frames = realloc(f->bone_frames, sizeof(bone_frame) * f->num_bone_frames);
  
  bone_frame bf;
  bf.bone_id = id;
  bf.position = pos;
  bf.rotation = rot;
  
  f->bone_frames[f->num_bone_frames-1] = bf;
}

animation* animation_new() {
  animation* a = malloc(sizeof(animation));
  a->speed = 60;
  a->start = 0;
  a->end = 0;
  a->num_frames = 0;
  a->frames = malloc(sizeof(frame*) * 0);
  return a;
}

animation* animation_identity(skeleton* s) {
  animation* a = animation_new();
  
  a->num_frames += 2;
  a->frames = realloc(a->frames, sizeof(frame*) * a->num_frames);
  a->start = 0;
  a->end = 1;
  
  frame* f0 = frame_new(0);
  frame* f1 = frame_new(1);
  
  int i;
  for(i = 0; i < s->num_bones; i++) {
    frame_add_bone_frame(f0, s->bones[i]->id, s->bones[i]->position, s->bones[i]->rotation);
    frame_add_bone_frame(f1, s->bones[i]->id, s->bones[i]->position, s->bones[i]->rotation);
  }
  
  a->frames[0] = f0;
  a->frames[1] = f1;
  
  return a;
}

void animation_delete(animation* a) {
  
  int i;
  for (i = 0; i < a->num_frames; a++) {
    frame_delete(a->frames[i]);
  }
  
  free(a);
}

frame* animation_new_frame(animation* a, float time) {
  a->num_frames ++;
  a->frames = realloc(a->frames, sizeof(frame*) * a->num_frames);
  a->frames[a->num_frames-1] = frame_new(time);
  return a->frames[a->num_frames-1];
}

static int state_load_empty = 0;
static int state_load_skeleton = 1;

animation* ani_load_file(char* filename) {
  
  char line[1024];
  
  char* c = asset_file_contents(filename);
  
  int i = 0;
  int j = 0;
  int state = state_load_empty;
  
  animation* a =  animation_new();
  frame* curr_frame = NULL;
  
  while(1) {
    if( c[i] == '\0') { break; }
    if( c[i] == '\n') {
    
      line[j-1] = '\0';
      
        /* Process line */
        if (state == state_load_empty) {
          
          int version;
          if (sscanf(line, "version %i", &version) > 0) {
            if (version != 1) {
              error("Can't load ani file %s. Don't know how to load version %i\n", filename, version);
            }
          }
          
          if (strcmp(line, "skeleton") == 0) {
            state = state_load_skeleton;
          }
        }
        
        else if (state == state_load_skeleton) {
          
          int time;
          if (sscanf(line, "time %i", &time) > 0) {
            curr_frame = animation_new_frame(a, time);
          }
        
          int id;
          float x, y, z, rx, ry, rz;
          if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) > 0) {
            vector3 position = v3(x, y, z);
            vector4 rotation = v4_quaternion_euler(rx, ry, rz);
            frame_add_bone_frame(curr_frame, id, position, rotation);
          }
          
          if (strcmp(line, "end") == 0) {
            state = state_load_empty;
          }
        }
        
        /* End Process line */
      
      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      line[j] = c[i];
      j++;
    
    }
    
    i++;
  }
  
  free(c);
  
  return a;
}
