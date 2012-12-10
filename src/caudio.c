#include "caudio.h"

static int audio_rate = 22050;
static Uint16 audio_format = AUDIO_S16;
static int audio_channels = 2;
static int audio_buffers = 4096;
static float volume = 1.0;

void audio_init() {
  
  int err;
  
  err = SDL_InitSubSystem(SDL_INIT_AUDIO);
  if (err == -1) { error("Could not start audio!"); }
  
  err = Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers);
  if(err == -1) { error("Unable to start audio mixer!"); }
  
  Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  
}

void audio_finish() {
  Mix_CloseAudio();
}

int audio_sound_play(sound* s, int loops) {
  int chan = Mix_PlayChannel(-1, s->sample, loops);
  if (chan == -1) { error("Unable to play sound: %s", Mix_GetError()); }
  return chan;
}

void audio_sound_pause(int channel) {
  Mix_Pause(channel);
}

void audio_sound_resume(int channel) {
  Mix_Resume(channel);
}

void audio_sound_stop(int channel) {
  Mix_HaltChannel(channel);
}

static const int fade_time = 5000;

void audio_music_play(music* m) {
  int err = Mix_FadeInMusic(m->handle, -1, fade_time);
  if (err == -1) { error("Unable to play music: '%s'", Mix_GetError()); }
}

void audio_music_pause() {
  Mix_PauseMusic();
}

void audio_music_resume() {
  Mix_ResumeMusic();
}

void audio_music_stop() {
  int err = Mix_FadeOutMusic(fade_time);
}


