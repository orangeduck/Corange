#ifndef audio_mixer_h
#define audio_mixer_h

#include "sound.h"

void audio_mixer_init();
void audio_mixer_finish();

void audio_mixer_disable();
void audio_mixer_enable();

bool audio_mixer_enabled();

void audio_mixer_set_volume(float vol);
float audio_mixer_get_volume();

void audio_mixer_play_sound(sound* s);

int audio_mixer_active_sounds();

#endif