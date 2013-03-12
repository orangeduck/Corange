/**
*** :: Animation ::
***  
***   Contains an array of frames and frame times.
***
**/

#ifndef animation_h
#define animation_h

#include "skeleton.h"

typedef struct {
  
  int frame_count;
  float* frame_times;
  frame** frames;

} animation;

animation* animation_new();
void animation_delete(animation* a);

frame* animation_new_frame(animation* a, float frametime, frame* base);

animation* ani_load_file(char* filename);

#endif