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

#endif
