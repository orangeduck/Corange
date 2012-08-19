/**
*** :: Animation ::
***  
***   Contains an array of frames (skeletons).
***   And frame times.
***
**/

#ifndef animation_h
#define animation_h

#include "skeleton.h"

typedef struct {

  float start_time, end_time;
  
  int num_frames;
  float* frame_times;
  skeleton** frames;

} animation;

animation* animation_new();
void animation_delete(animation* a);

skeleton* animation_new_frame(animation* a, float frametime, skeleton* base);

animation* ani_load_file(char* filename);

#endif