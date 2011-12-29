#include <stdio.h>

#include "platformer.h"

void platformer_init() {
  
  /* Set the screen clear color and depth value */
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  
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
  
  /* Clear the screen to a single color */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  /* Flip the Screen Buffer */
  SDL_GL_SwapBuffers(); 
  
}

void platformer_finish() {
  
}

int main(int argc, char **argv) {
  
  /* Init Corange, pointing to the core_assets folder */
  corange_init("../../core_assets");
  
  /* Init platformer game */
  platformer_init();
  
  /* Set the game running, create SDL_Event */
  bool running = 1;
  SDL_Event event;
  
  while(running) {
    
    /* Frame functions used to monitor frame times, FPS and others */
    frame_begin();
    
    /* Poll SDL Event */
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        /* Exit on ESCAPE and Screenshot on print screen */
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = false; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
      
      /* Run Platformer Events */
      platformer_event(event);
    }
    
    /* Run Platformer Game Update */
    platformer_update();
    
    /* Render Platformer Game */
    platformer_render();
    
    frame_end();
  }
  
  /* Finish Platformer Game */
  platformer_finish();
  
  /* Finish Corange. This will unload and assets and delete any remaining entities */
  corange_finish();
  
  return 0;
}
