#ifndef audio_mixer_h
#define audio_mixer_h

#include "sound.h"

void audio_mixer_init();
void audio_mixer_finish();

void audio_mixer_play_sound(sound* s);

int audio_mixer_active_sounds();

#endif