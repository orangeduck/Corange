#include <math.h>

#include "error.h"
#include "bool.h"
#include "vector.h"

#include "sound.h"

#include "audio_mixer.h"

static SDL_AudioSpec system_spec;
static bool enabled = true;
static float volume = 1.0f;

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
  
  int i, j;
  for(i = 0; i < samples_num; i++) {
    for(j = 0; j < MAX_CHANNELS; j++) {
      
      if (!channels[j].active) continue;
      
      uint16_mono_t* snd_samples = (uint16_mono_t*)channels[j].sound->data;
      int snd_len = channels[j].sound->length / sizeof(uint16_mono_t);
      
      if(channels[j].position >= snd_len) {
        channels[j].active = false;
        break;
      }
      
      if (enabled) {
        samples[i].left += snd_samples[channels[j].position];
        samples[i].right += snd_samples[channels[j].position];
      } else {
        samples[i].left = 0;
        samples[i].right = 0;
      }
      
      channels[j].position++;
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
  
  bool slot_free = false;
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    if (!channels[i].active) {
      
      slot_free = true;
      
      channels[i].sound = s;
      channels[i].position = 0;
      channels[i].left_volume = 1.0;
      channels[i].right_volume = 1.0;
      channels[i].loops = 0;
      
      channels[i].active = true;
      break;
    }
  }
  
  if (!slot_free) {
    warning("Did not play sound. Reached maximum number of %i active sounds.", MAX_CHANNELS);
  }
  
}

int audio_mixer_active_sounds() {
  
  int count = 0;
  
  int i;
  for(i = 0; i < MAX_CHANNELS; i++) {
    if(channels[i].active) { count++; }
  }
  
  return count;
  
}

void audio_mixer_disable() {
  enabled = false;
}

void audio_mixer_enable() {
  enabled = true;
}

bool audio_mixer_enabled() {
  return enabled;
}

void audio_mixer_set_volume(float vol) {
  volume = vol;
}

float audio_mixer_get_volume() {
  return volume;
}