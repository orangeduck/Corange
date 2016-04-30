#include "cjoystick.h"

#define MAX_STICKS 8

static int num_sticks = 0;
SDL_Joystick* sticks[MAX_STICKS];

void joystick_init() {
  
  int error = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  if (error == -1) {
    error("Cannot initialize SDL joystick!");
  }
  
  num_sticks = SDL_NumJoysticks();
  debug("Found %i joysticks", num_sticks);
  
  for(int i = 0; i < num_sticks; i++) {
    sticks[i] = SDL_JoystickOpen(i);
    
    if (sticks[i] == NULL) {
      error("Couldn't open joystick %i!", i);
    } else {
      debug("JoyStick %i (%s) loaded.", i, SDL_JoystickName(sticks[i]));
    }
  }
  
}

void joystick_finish() {

  for(int i = 0; i < num_sticks; i++) {
    SDL_JoystickClose(sticks[i]);
  }
  
}


int joystick_count() {
  return num_sticks;
}

SDL_Joystick* joystick_get(int i) {
  if (i >= num_sticks) {
    error("Unable to get Joystick at index %i. Only have %i joysticks.", i, num_sticks);
  }
  
  return sticks[i];
}
