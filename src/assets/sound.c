#include "assets/sound.h"

sound* wav_load_file(char* filename) {
  sound* s = malloc(sizeof(sound));
  s->sample = Mix_LoadWAV(filename);
  
  if (!s->sample) { error("Couldn't load sound '%s' : %s", filename, Mix_GetError()); }
  
  return s;
}

void sound_delete(sound* s) {
  Mix_FreeChunk(s->sample);
  free(s);
}

int sound_play(sound* s) {
  return sound_play_looped(s, 0);
}

int sound_play_looped(sound* s, int loops) {
  return Mix_PlayChannel(-1, s->sample, loops);
}

int sound_play_at(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir) {
  return sound_play_at_looped(s, pos, cam_pos, cam_dir, 0);
}



int sound_play_at_looped(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir, int loops) {
  int c = sound_play_looped(s, loops);
  
  const float HEARING = 5;
  
  float distance = vec3_dist(pos, cam_pos);
  Uint8 dist_val = (Uint8)clamp(distance * HEARING, 0, 255); 
  
  const float DEGREES = 57.2957795;
  
  vec3 to_position = vec3_normalize(vec3_sub(pos, cam_pos));
  vec3 to_forward  = vec3_normalize(cam_dir);
  float angle = acos(vec3_dot(to_position, to_forward));
  Sint16 angle_val = DEGREES * angle;
  
  Mix_SetPosition(c, angle_val, dist_val);
  return c;
}