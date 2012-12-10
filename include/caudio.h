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
#include "assets/music.h"

void audio_init();
void audio_finish();

int audio_sound_play(sound* s, int loops);
void audio_sound_pause(int channel);
void audio_sound_resume(int channel);
void audio_sound_stop(int channel);

void audio_music_play(music* m);
void audio_music_pause();
void audio_music_resume();
void audio_music_stop();

void audio_music_set_volume(float volume);
float audio_music_get_volume();

#endif
