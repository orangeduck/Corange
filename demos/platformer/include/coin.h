#ifndef coin_h
#define coin_h

#include "corange.h"

typedef struct {
  vec2 position;
} coin;

coin* coin_new();
void coin_delete(coin* c);

void coin_render(coin* c, vec2 camera_position);


#endif