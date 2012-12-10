#include "assets/sound.h"

sound* wav_load_file(char* filename) {
  sound* s = malloc(sizeof(sound));
  s->sample = Mix_LoadWAV(filename);
  
  if (!s->sample) { error("Couldn't load sound '%s' : %s", filename, Mix_GetError()); }
  
  return s;
}

void sound_delete(sound* s) {
  Mix_FreeChunk(s->sample);
  free(s);
}
