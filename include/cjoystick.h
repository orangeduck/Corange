#ifndef cjoystick_h
#define cjoystick_h

#include "cengine.h"

void joystick_init();
void joystick_finish();

int joystick_count();
SDL_Joystick* joystick_get(int i);

#endif