/**
*** :: Music ::
***
***   Can be loaded from WAV
***
**/

#ifndef music_h
#define music_h

#include "cengine.h"

typedef struct {
  Mix_Music* handle;
} music;

music* mp3_load_file(char* filename);
music* ogg_load_file(char* filename);
void music_delete(music* m);

#endif
