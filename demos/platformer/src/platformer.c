#include <stdio.h>

#include "platformer.h"

void platformer_init() {

  viewport_set_vsync(1);

}

void platformer_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
        
  break;

  case SDL_MOUSEBUTTONDOWN:
    
  break;
  
  case SDL_MOUSEMOTION:

  break;
  }
    
}

void platformer_update() {
  

}

void platformer_render() {
  
  SDL_GL_SwapBuffers(); 
  
}

void platformer_finish() {
  
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  platformer_init();
  
  int running = 1;
  SDL_Event event;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      platformer_event(event);
    }
    
    platformer_update();
    
    platformer_render();
    
    frame_end();
  }
  
  platformer_finish();
  
  corange_finish();
  
  return 0;
}
