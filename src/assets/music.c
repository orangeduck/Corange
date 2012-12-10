
#include "assets/music.h"

music* mp3_load_file(char* filename) {
  music* m = malloc(sizeof(music));
  m->handle = Mix_LoadMUS(filename);
  if (!m->handle) { error("Couldn't load music '%s' : %s", filename, Mix_GetError()); }
  return m;
}

music* ogg_load_file(char* filename) {
  music* m = malloc(sizeof(music));
  m->handle = Mix_LoadMUS(filename);
  if (!m->handle) { error("Couldn't load music '%s' : %s", filename, Mix_GetError()); }
  return m;
}

void music_delete(music* m) {
  Mix_FreeMusic(m->handle);
  free(m);
}
