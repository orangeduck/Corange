/**
*** :: Audio ::
***
***   Manages the playing of sounds.
***
**/

#ifndef caudio_h
#define caudio_h

#include "cengine.h"
#include "assets/sound.h"

void audio_init();
void audio_finish();

void audio_disable();
void audio_enable();
bool audio_enabled();

void audio_set_volume(float vol);
float audio_get_volume();

void audio_play_sound(sound* s);
int audio_active_sounds();

#endif
