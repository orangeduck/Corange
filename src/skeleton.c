#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "asset_manager.h"

#include "skeleton.h"

skeleton* skeleton_new() {
  skeleton* s =  malloc(sizeof(skeleton));
  s->num_bones = 0;
  s->bones = malloc(sizeof(bone*) * s->num_bones);
  return s;
}

void skeleton_delete(skeleton* s) {
  int i;
  for(i = 0; i < s->num_bones; i++) {
    bone_delete(s->bones[i]);
  }
  free(s->bones);
  free(s);
}

void skeleton_add_bone(skeleton* s, char* name, int id, int parent_id) {
  
  bone* b = bone_new(id, name);
  b->parent = skeleton_bone_id(s, parent_id);
  
  s->num_bones++;
  s->bones = realloc(s->bones, sizeof(bone*) * s->num_bones);
  s->bones[s->num_bones-1] = b;
  
}

void skeleton_print(skeleton* s) {
  int i;
  for(i = 0; i < s->num_bones; i++) {
    bone* b = s->bones[i];
    printf("Bone %i: %i %s ", i, b->id, b->name);
    v3_print(b->position);printf(" ");
    v4_print(b->rotation);
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
  
  int i;
  for(i = 0; i < s->num_bones; i++) {
    if (s->bones[i]->id == id) {
      return s->bones[i];
    }
  }
  
  warning("unknown bone parent id %i", id);
  return NULL;
}

bone* bone_new(int id, char* name) {
  bone* b = malloc(sizeof(bone));
  b->name = malloc(strlen(name) + 1);
  strcpy(b->name, name);
  b->id = id;
  b->position = v3_zero();
  b->rotation = v4_quaternion_id();
  return b;
}

void bone_delete(bone* b) {
  free(b->name);
  free(b);
}

static int state_load_empty = 0;
static int state_load_nodes = 1;
static int state_load_skeleton = 2;

skeleton* skl_load_file(char* filename) {
  
  char line[1024];
  
  char* c = asset_file_contents(filename);
  
  int i = 0;
  int j = 0;
  int state = state_load_empty;
  
  skeleton* s =  skeleton_new();
  
  while(1) {
    if( c[i] == '\0') { break; }
    if( c[i] == '\n') {
    
      line[j-1] = '\0';
      
        /* Process line */
        if (state == state_load_empty) {
          
          int version;
          if (sscanf(line, "version %i", &version) > 0) {
            if (version != 1) {
              error("Can't load skl file %s. Don't know how to load version %i\n", filename, version);
            }
          }
          
          if (strcmp(line, "nodes") == 0) {
            state = state_load_nodes;
          }
          
          if (strcmp(line, "skeleton") == 0) {
            state = state_load_skeleton;
          }
        }
        
        else if (state == state_load_nodes) {
          char name[1024];
          int id, parent_id;
          if (sscanf(line, "%i %s %i", &id, name, &parent_id) > 0) {
            /* Bone name might well contain quotation marks. Cant be bothered to remove atm. */
            skeleton_add_bone(s, name, id, parent_id);
          }
          
          if (strcmp(line, "end") == 0) {
            state = state_load_empty;
          }
        }
        
        else if (state == state_load_skeleton) {
          int id;
          float x, y, z, rx, ry, rz;
          if (sscanf(line, "%i %f %f %f %f %f %f", &id, &x, &y, &z, &rx, &ry, &rz) > 0) {
            bone* b = skeleton_bone_id(s, id);
            b->position = v3(x, y, z);
            b->rotation = v4_quaternion_euler(rx, ry, rz);
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
  
  return s;
}
