#include <stdio.h>

#include "level.h"
#include "character.h"

#include "platformer.h"

/* Store current level */
static level* current_level = NULL;

/* store Camera position */
static vector2 camera_position;

/* Some booleans to monitor key inputs */
static bool left_held = false;
static bool right_held = false;
static bool up_held = false;
static bool down_held = false;

void platformer_init() {
  
  /* Load textures in tiles folder */
  load_folder("./tiles/");
  
  /* Register some functions for loading/unloading levels */
  asset_manager_handler("level", level_load_file, level_delete);
  
  /* Load levels from levels folder */
  load_folder("./levels/");
  
  /* Register some handler for entity types */
  entity_manager_handler(character, character_new, character_delete);
  
  /* New main character entity */
  character* main_char = entity_new("main_char", character);
  main_char->position = v2(400, -400);
  
  current_level = asset_get("./levels/demo.level");
  
  /* Set the screen clear color and depth value */
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  
}

void platformer_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = true; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = true; }
    if (event.key.keysym.sym == SDLK_UP) { up_held = true; }
    if (event.key.keysym.sym == SDLK_DOWN) { down_held = true; }
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = false; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = false; }
    if (event.key.keysym.sym == SDLK_UP) { up_held = false; }
    if (event.key.keysym.sym == SDLK_DOWN) { down_held = false; }
  break;

  case SDL_MOUSEBUTTONDOWN:
    
  break;
  
  case SDL_MOUSEMOTION:

  break;
  }
    
}

void platformer_update() {
  
  character* main_char = entity_get("main_char");
  
  const float char_speed = 0.001;
  
  if (left_held) {
    main_char->velocity.x -= char_speed;
  } else if (right_held) {
    main_char->velocity.x += char_speed;
  } else if (up_held) {
    main_char->velocity.y += char_speed;
  } else if (down_held) {
    main_char->velocity.y -= char_speed;
  } else {
    main_char->velocity = v2_mul(main_char->velocity, 0.99);
  }
  
  character_update(main_char);
  
  /* Camera follows main character */
  camera_position = main_char->position;

}

void platformer_render() {
  
  /* Clear the screen to a single color */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  /* Render level */
  level_render(current_level, camera_position);
  
  /* Render main character */
  character_render(entity_get_as("main_char", character), camera_position);
  
  /* Flip the Screen Buffer */
  SDL_GL_SwapBuffers(); 
  
}

void platformer_finish() {
  
  /* Entity and asset managers will automatically delete any remaining objects */
  
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
