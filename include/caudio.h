/**
*** :: Audio ::
***
***   Basic Audio layer.
***
***   Currently only plays flat sounds.
***   No support for source manipulation.
***   Could do with some more love.
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
