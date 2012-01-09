#include <stdio.h>
#include <math.h>

#include "level.h"
#include "character.h"
#include "coin.h"

#include "platformer.h"

/* Store current level */
static level* current_level = NULL;

/* store Camera position */
static vector2 camera_position;

/* Some booleans to monitor key inputs */
static bool left_held = false;
static bool right_held = false;

void platformer_init() {
  
  /* Register some functions for loading/unloading levels */
  asset_manager_handler("level", level_load_file, level_delete);
  
  /* Load Assets */
  load_folder("./tiles/");
  load_folder("./backgrounds/");
  load_folder("./sounds/");
  load_folder("./levels/");
  
  current_level = asset_get("./levels/demo.level");
  
  /* Register some handlers for entity types */
  entity_manager_handler(character, character_new, character_delete);
  entity_manager_handler(coin, coin_new, coin_delete);
  
  /* New main character entity */
  character* main_char = entity_new("main_char", character);
  main_char->position = v2_mul( v2(20, 20), 32);
  
  /* Some Coins */
  entities_new("c%i", 10, coin);
  entity_get_as("c0", coin)->position = v2_mul(v2(18, 18), 32);
  entity_get_as("c1", coin)->position = v2_mul(v2(33, 28), 32);
  entity_get_as("c2", coin)->position = v2_mul(v2(41, 22), 32);
  entity_get_as("c3", coin)->position = v2_mul(v2(20, 17), 32);
  entity_get_as("c4", coin)->position = v2_mul(v2(18, 28), 32);
  entity_get_as("c5", coin)->position = v2_mul(v2(36, 20), 32);
  entity_get_as("c6", coin)->position = v2_mul(v2(20, 10), 32);
  entity_get_as("c7", coin)->position = v2_mul(v2(31, 16), 32);
  entity_get_as("c8", coin)->position = v2_mul(v2(45, 23), 32);
  entity_get_as("c9", coin)->position = v2_mul(v2(49, 26), 32);
  
  /* Set the screen clear color and depth value */
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  
  /* Add some UI elements */
  ui_text* framerate_text = ui_elem_new("framerate_text", ui_text);
  framerate_text->position = v2(10, 10);
  framerate_text->color = v4_white();
  ui_text_update_string(framerate_text, "framerate");
  
  ui_text* audio_text = ui_elem_new("audio_text", ui_text);
  audio_text->position = v2(10, 30);
  audio_text->color = v4_white();
  ui_text_update_string(audio_text, "Audio Enabled");
  
}

void platformer_event(SDL_Event event) {
  
  character* main_char = entity_get("main_char");
  
  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = true; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = true; }
    
    if (event.key.keysym.sym == SDLK_UP) {
      main_char->velocity.y -= 5.0;
    }
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = false; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = false; }
    
    if (event.key.keysym.sym == SDLK_p) { v2_print( v2_div(main_char->position, 32)); }
    
    if (event.key.keysym.sym == SDLK_w) { camera_position.y += 10.0; }
    if (event.key.keysym.sym == SDLK_s) { camera_position.y -= 10.0; }
    if (event.key.keysym.sym == SDLK_d) { camera_position.x += 10.0; }
    if (event.key.keysym.sym == SDLK_a) { camera_position.x -= 10.0; }
  break;
  
  }
  
  ui_event(event);
    
}

static void collision_detection() {

  character* main_char = entity_get("main_char");
  
  const float buffer = 4;
  const float bounce = 0.5;
  
  vector2 diff;
  
  /* Bottom Collision */
  
  diff = v2_fmod(main_char->position, 32);
  
  vector2 bottom1 = v2_add(main_char->position, v2(buffer, 32));
  vector2 bottom2 = v2_add(main_char->position, v2(32 - buffer, 32));
  
  bool bottom1_col = tile_has_collision(level_tile_at(current_level, bottom1));
  bool bottom2_col = tile_has_collision(level_tile_at(current_level, bottom2));
  
  if (bottom1_col || bottom2_col) {
    main_char->position = v2_add(main_char->position, v2(0,-diff.y));
    main_char->velocity.y *= -bounce;
  }
  
  /* Top Collision */
  
  diff = v2_fmod(main_char->position, 32);
  
  vector2 top1 = v2_add(main_char->position, v2(buffer, 0));
  vector2 top2 = v2_add(main_char->position, v2(32 - buffer, 0));
  
  bool top1_col = tile_has_collision(level_tile_at(current_level, top1));
  bool top2_col = tile_has_collision(level_tile_at(current_level, top2));
  
  if (top1_col || top2_col) {
    main_char->position = v2_add(main_char->position, v2(0, 32 - diff.y));
    main_char->velocity.y *= -bounce;
  }
  
  /* Left Collision */
  
  diff = v2_fmod(main_char->position, 32);
  
  vector2 left1 = v2_add(main_char->position, v2(0, buffer));
  vector2 left2 = v2_add(main_char->position, v2(0, 32 - buffer));
  
  bool left1_col = tile_has_collision(level_tile_at(current_level, left1));
  bool left2_col = tile_has_collision(level_tile_at(current_level, left2));
  
  if (left1_col || left2_col) {
    main_char->position = v2_add(main_char->position, v2(32 - diff.x,0));
    main_char->velocity.x *= -bounce;
  }
  
  /* Right Collision */
  
  diff = v2_fmod(main_char->position, 32);
  
  vector2 right1 = v2_add(main_char->position, v2(32, buffer));
  vector2 right2 = v2_add(main_char->position, v2(32, 32 - buffer));
  
  bool right1_col = tile_has_collision(level_tile_at(current_level, right1));
  bool right2_col = tile_has_collision(level_tile_at(current_level, right2));
  
  if (right1_col || right2_col) {
    main_char->position = v2_add(main_char->position, v2(-diff.x,0));
    main_char->velocity.x *= -bounce;
  }
  
}

static void collision_detection_coins() {
  
  character* main_char = entity_get("main_char");
  
  vector2 top_left = v2_add(main_char->position, v2(-32, -32));
  vector2 bottom_right = v2_add(main_char->position, v2(32, 32));
  
  coin* coins[512];
  int num_coins = 0;
  entities_get(coins, &num_coins, coin); 
  
  int i;
  for(i = 0; i < num_coins; i++) {
    char* coin_name = entity_name(coins[i]);
  
    if ((coins[i]->position.x > top_left.x) &&
        (coins[i]->position.x < bottom_right.x) &&
        (coins[i]->position.y > top_left.y) && 
        (coins[i]->position.y < bottom_right.y)) {
          
          debug("Got Coin %s!", coin_name);
          audio_mixer_play_sound(asset_get_as("./sounds/coin.wav", sound));
          entity_delete(coin_name);
        }
  }
  
}

void platformer_update() {
  
  character* main_char = entity_get("main_char");
  
  const float roll_speed = 0.1;
  
  if (left_held) {
    main_char->velocity.x -= roll_speed;
  } else if (right_held) {
    main_char->velocity.x += roll_speed;
  } else {
    main_char->velocity.x *= 0.95;
  }
  
  const float gravity = 0.2;
  main_char->velocity.y += gravity;
  
  character_update(main_char);
  
  collision_detection();
  collision_detection_coins();
  
  /* Camera follows main character */
  camera_position = v2(main_char->position.x, -main_char->position.y);
  
  /* Update the framerate text */
  ui_text* framerate_text = ui_elem_get("framerate_text");
  ui_text_update_string(framerate_text, frame_rate_string());
  
  ui_update();
  
}

void platformer_render() {
  
  /* Clear the screen to a single color */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  level_render_background(current_level);
  
  /* Render main character */
  character_render(entity_get_as("main_char", character), camera_position);
  
  coin* coins[512];
  int num_coins = 0;
  entities_get(coins, &num_coins, coin); 
  
  int i;
  for(i = 0; i < num_coins; i++) {
    coin_render(coins[i], camera_position);
  }
  
  /* Render level */
  level_render_tiles(current_level, camera_position);
  
  /* Render UI */
  ui_render();
  
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
        break;
      case SDL_KEYUP:
        /* Exit on ESCAPE and Screenshot on print screen */
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = false; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        if (event.key.keysym.sym == SDLK_a) {
          ui_text* audio_text = ui_elem_get("audio_text");
          if(audio_mixer_enabled()) {
            audio_mixer_disable();
            debug("Audio Disabled");
            ui_text_update_string(audio_text, "Audio Disabled");
          } else {
            audio_mixer_enable();
            debug("Audio Enabled");
            ui_text_update_string(audio_text, "Audio Enabled");
          }
        }
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
    
    //frame_end();
    frame_end_at_rate(60);
  }
  
  /* Finish Platformer Game */
  platformer_finish();
  
  /* Finish Corange. This will unload and assets and delete any remaining entities */
  corange_finish();
  
  return 0;
}
