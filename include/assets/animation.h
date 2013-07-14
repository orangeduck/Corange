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
  float frame_time;
  frame** frames;

} animation;

animation* animation_new();
void animation_delete(animation* a);
float animation_duration(animation* a);

frame* animation_add_frame(animation* a, frame* base);
frame* animation_frame(animation* a, int i);
frame* animation_sample(animation* a, float time);
void animation_sample_to(animation* a, float time, frame* out);

animation* ani_load_file(char* filename);

#endif