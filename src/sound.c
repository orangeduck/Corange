#include "error.h"

#include "sound.h"

static void flip_endian(char* data, int length) {
	for(int i = 0; i < length; i += 2) {
		int x = data[i];
		data[i] = data[i + 1];
		data[i + 1] = x;
	}
}

sound* wav_load_file(char* filename) {
  
  sound* s = malloc(sizeof(sound));
  
	SDL_AudioSpec spec;
  
	if( SDL_LoadWAV(filename, &spec, (Uint8**)&s->data, (Uint32*)&s->length) == NULL) {
    error("Unable to load sound file %s", filename);
  }
  
	if (spec.freq != 44100) {
		warning("Sound file %s is %f, not 44.1 kHz. May sound incorrect", filename, (spec.freq)/1000.0f);
  }
  
	if (spec.channels != 1) {
		warning("Sound file %s has %i channels. Currently only mono sounds supported.", filename, spec.channels);
	}
  
  if ((spec.format != AUDIO_S16LSB) &&
      (spec.format != AUDIO_S16MSB)) {
    error("Unsupported sound format for file %s, id %i.", filename, spec.format);
  }
  
  if (spec.format != AUDIO_S16SYS) {
    flip_endian(s->data, s->length);
  }
  
  return s;
}

void sound_delete(sound* s) {
  SDL_FreeWAV((Uint8*)s->data);
  free(s);
}