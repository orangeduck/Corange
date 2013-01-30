/**
*** :: Sound ::
***
***   Can be loaded from WAV
***
**/

#ifndef sound_h
#define sound_h

#include "cengine.h"

typedef struct {
  Mix_Chunk* sample;
} sound;

sound* wav_load_file(char* filename);
void sound_delete(sound* s);

int sound_play(sound* s);
int sound_play_looped(sound* s, int loops);

int sound_play_at(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir);
int sound_play_at_looped(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir, int loops);

#endif
