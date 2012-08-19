/**
*** :: Audio Manager ::
***
***   Manages the playing of sounds.
***
**/

#ifndef audio_manager_h
#define audio_manager_h

#include "cengine.h"
#include "assets/sound.h"

void audio_manager_init();
void audio_manager_finish();

void audio_disable();
void audio_enable();
bool audio_enabled();

void audio_set_volume(float vol);
float audio_get_volume();

void audio_manager_play_sound(sound* s);
int audio_manager_active_sounds();

#endif
