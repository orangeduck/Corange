#include "error.h"
#include "bool.h"
#include "vector.h"
#include <math.h>

#include "sound.h"

#include "audio_mixer.h"

static SDL_AudioSpec system_spec;

typedef struct {
  bool active;
	sound* sound;
	int	position;
	float	left_volume;
	float right_volume;
  int loops;
} sound_channel;

#define MAX_CHANNELS 8
sound_channel channels[MAX_CHANNELS];

typedef struct {
  uint16_t left;
  uint16_t right;
} uint16_stereo_t;

typedef uint16_t uint16_mono_t;

static void audio_mixer_mix(void* unused, char* stream, int stream_size) {
  
  uint16_stereo_t* samples = (uint16_stereo_t*)stream;
  int samples_num = stream_size / sizeof(uint16_stereo_t);
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    
    if (!channels[i].active) {
      continue;
    }
    
    uint16_mono_t* snd_samples = (uint16_mono_t*)channels[i].sound->data;
    int snd_len = channels[i].sound->length / sizeof(uint16_mono_t);
    
    int j;
    for(j = 0; j < samples_num; j++) {
      
      if(channels[i].position >= snd_len) {
        channels[i].active = false;
        break;
      }
      
      samples[j].left += snd_samples[channels[i].position];
      samples[j].right += snd_samples[channels[i].position];
      
      channels[i].position++;
    }
  
  }
  
}

void audio_mixer_init() {
  
	SDL_AudioSpec as;
	as.freq = 44100;
	as.format = AUDIO_S16SYS;
	as.channels = 2;
	as.samples = 1024;
	as.callback = (void(*)(void*,Uint8*,int))audio_mixer_mix;
  
	if(SDL_OpenAudio(&as, &system_spec) < 0) {
    error("Cannot start audio");
  }

	if(system_spec.format != AUDIO_S16SYS) {
    error("System audio spec must be AUDIO_S16SYS");
  }
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    channels[i].active = false;
  }
  
	SDL_PauseAudio(0);
  
}

void audio_mixer_finish() {
  
	SDL_PauseAudio(1);
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    channels[i].active = false;
  }
    
	SDL_CloseAudio();
  
}

void audio_mixer_play_sound(sound* s) {
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    
    if (!channels[i].active) {
      
      channels[i].sound = s;
      channels[i].position = 0;
      channels[i].left_volume = 1.0;
      channels[i].right_volume = 1.0;
      channels[i].loops = 0;
      
      channels[i].active = true;
      break;
    }
  }
  
}