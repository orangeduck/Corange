#include <math.h>

#include "error.h"
#include "vector.h"

#include "audio_manager.h"

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
  int16_t left;
  int16_t right;
} int16_stereo_t;

typedef int16_t int16_mono_t;

static float volume_to_amplitude(float vol) {
  return pow(10, vol) / 10;
}

static void audio_mix(void* unused, char* stream, int stream_size) {
  
  int16_stereo_t* samples = (int16_stereo_t*)stream;
  int samples_num = stream_size / sizeof(int16_stereo_t);
  
  int i, j;
  for(i = 0; i < samples_num; i++) {
    for(j = 0; j < MAX_CHANNELS; j++) {
      
      if (!channels[j].active) continue;
      
      int16_mono_t* snd_samples = (int16_mono_t*)channels[j].sound->data;
      int snd_len = channels[j].sound->length / sizeof(int16_mono_t);
      
      if(channels[j].position >= snd_len) {
        channels[j].active = false;
        break;
      }
      
      if (enabled) {
        double left = snd_samples[channels[j].position] / 32768.0;
        double right = snd_samples[channels[j].position] / 32768.0;
        
        left = clamp(left * volume_to_amplitude(volume), -1, 1);
        right = clamp(right * volume_to_amplitude(volume), -1, 1);
        
        samples[i].left += left * 32768;
        samples[i].right += right * 32768;
      } else {
        samples[i].left = 0;
        samples[i].right = 0;
      }
      
      channels[j].position++;
    }
  }
  
}

void audio_manager_init() {
  
  int error = SDL_Init(SDL_INIT_AUDIO);
  if (error == -1) {
    error("Cannot start SDL audio!");
  }  

  SDL_AudioSpec as;
  as.freq = 44100;
  as.format = AUDIO_S16SYS;
  as.channels = 2;
  as.samples = 1024;
  as.callback = (void(*)(void*,Uint8*,int))audio_mix;
  
  if(SDL_OpenAudio(&as, &system_spec) < 0) {
    error("Cannot start audio");
  }

	if(system_spec.format != AUDIO_S16SYS) {
    error("System audio spec must be AUDIO_S16SYS");
  }
  
  for(int i = 0; i < MAX_CHANNELS; i++) {
    channels[i].active = false;
  }
  
	SDL_PauseAudio(0);
  
}

void audio_manager_finish() {
  
	SDL_PauseAudio(1);
  
  for(int i = 0; i < MAX_CHANNELS; i++) {
    channels[i].active = false;
  }
    
	SDL_CloseAudio();
  
}

void audio_manager_play_sound(sound* s) {
  
  bool slot_free = false;
  
  for(int i = 0; i < MAX_CHANNELS; i++) {
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

int audio_manager_active_sounds() {
  
  int count = 0;
  
  for(int i = 0; i < MAX_CHANNELS; i++) {
    if(channels[i].active) { count++; }
  }
  
  return count;
  
}

void audio_disable() {
  enabled = false;
}

void audio_enable() {
  enabled = true;
}

bool audio_enabled() {
  return enabled;
}

void audio_set_volume(float vol) {
  volume = clamp(vol, 0, 1);
}

float audio_get_volume() {
  return volume;
}
