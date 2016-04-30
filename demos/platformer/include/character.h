#ifndef character_h
#define character_h

#include "corange.h"

typedef struct {
  vec2 velocity;
  vec2 position;
  float flap_timer;
  bool facing_left;
} character;

character* character_new();
void character_delete(character* c);

void character_update(character* c);
void character_render(character* c, vec2 camera_position);

#endif