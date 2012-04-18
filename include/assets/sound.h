#ifndef sound_h
#define sound_h

#include "SDL/SDL.h"

typedef struct {
  char* data;
  int length;
} sound;

sound* wav_load_file(char* filename);
void sound_delete(sound* s);

#endif
