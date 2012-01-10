#ifndef character_h
#define character_h

#include "corange.h"

typedef struct {
  vector2 velocity;
  vector2 position;
  float flap_timer;
  bool facing_left;
} character;

character* character_new();
void character_delete(character* c);

void character_update(character* c);
void character_render(character* c, vector2 camera_position);

#endif