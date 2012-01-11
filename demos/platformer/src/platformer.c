#include <stdio.h>
#include <math.h>

#include "level.h"
#include "character.h"
#include "coin.h"

#include "platformer.h"

static level* current_level = NULL;

static vector2 camera_position;

/* Some booleans to monitor key inputs */
static bool left_held = false;
static bool right_held = false;

static int level_score = 0;
static float level_time = 0.0;

/* We store all the coin positions here */
#define COIN_COUNT 45
static vector2 coin_positions[COIN_COUNT] = {
  {18, 18}, {33, 28}, {41, 22}, {20, 17}, {18, 28},
  {36, 20}, {20, 10}, {31, 16}, {45, 23}, {49, 26},
  {25, 18}, {20, 37}, {44, 32}, {66, 20}, {52, 20},
  {63, 11}, {52, 12}, {39, 13}, {27, 11}, {73, 20},
  {65, 29}, {72, 29}, {78, 30}, {78, 20}, {83, 22},
  {87, 22}, {90, 24}, {94, 19}, {99, 18}, {82, 13},
  {79, 14}, {106, 22}, {102, 30}, {100, 35}, {93, 27},
  {88, 34}, {98, 40}, {96, 40}, {94, 40}, {86, 40},
  {81, 37}, {77, 38}, {72, 34}, {65, 38}, {71, 37}
};

void platformer_init() {
  
  /* Register functions for loading/unloading files with the extension .level */
  asset_manager_handler("level", level_load_file, level_delete);
  
  /* Load Assets */
  load_folder("./tiles/");
  load_folder("./backgrounds/");
  load_folder("./sounds/");
  load_folder("./levels/");
  
  current_level = asset_get("./levels/demo.level");
  
  /* Register some handlers for creating and destroying entity types */
  entity_manager_handler(character, character_new, character_delete);
  entity_manager_handler(coin, coin_new, coin_delete);
  
  /* New main character entity */
  character* main_char = entity_new("main_char", character);
  main_char->position = v2_mul( v2(20, 20), 32);
  
  /* We can create multiple entities using a name format string like printf */
  entities_new("coin_id_%i", COIN_COUNT, coin);
  
  
  /* Get an array of pointers to all coin entities */
  coin* coins[COIN_COUNT];
  entities_get(coins, NULL, coin);
  
  /* Set all the coin initial positions */
  int i;
  for(i = 0; i < COIN_COUNT; i++) {
    coins[i]->position = v2_mul(coin_positions[i], 32);
  }
  
  /* Set the screen clear color and depth value */
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  
  /* Add some UI elements */
  ui_rectangle* framerate_rect = ui_elem_new("framerate_rect", ui_rectangle);
  framerate_rect->top_left = v2(10, 10);
  framerate_rect->bottom_right = v2(40, 35);
  framerate_rect->color = v4_black();
  framerate_rect->border_color = v4_white();
  framerate_rect->border_size = 1;
  
  ui_rectangle* audio_rect = ui_elem_new("audio_rect", ui_rectangle);
  audio_rect->top_left = v2(50, 10);
  audio_rect->bottom_right = v2(170, 35);
  audio_rect->color = v4_black();
  audio_rect->border_color = v4_white();
  audio_rect->border_size = 1;
  
  ui_rectangle* score_rect = ui_elem_new("score_rect", ui_rectangle);
  score_rect->top_left = v2(180, 10);
  score_rect->bottom_right = v2(300, 35);
  score_rect->color = v4_black();
  score_rect->border_color = v4_white();
  score_rect->border_size = 1;
  
  ui_rectangle* time_rect = ui_elem_new("time_rect", ui_rectangle);
  time_rect->top_left = v2(310, 10);
  time_rect->bottom_right = v2(420, 35);
  time_rect->color = v4_black();
  time_rect->border_color = v4_white();
  time_rect->border_size = 1;
  
  ui_text* framerate_text = ui_elem_new("framerate_text", ui_text);
  framerate_text->position = v2(17, 15);
  framerate_text->color = v4_white();
  ui_text_update_string(framerate_text, "framerate");
  
  ui_text* audio_text = ui_elem_new("audio_text", ui_text);
  audio_text->position = v2(60, 15);
  audio_text->color = v4_white();
  ui_text_update_string(audio_text, "Audio Enabled");
  
  ui_text* score_text = ui_elem_new("score_text", ui_text);
  score_text->position = v2(190, 15);
  score_text->color = v4_white();
  ui_text_update_string(score_text, "Score 000000");
  
  ui_text* time_text = ui_elem_new("time_text", ui_text);
  time_text->position = v2(320, 15);
  time_text->color = v4_white();
  ui_text_update_string(time_text, "Time 000000");
  
  /* Set volume to something more reasonable */
  audio_mixer_set_volume(0.25);
}

void platformer_event(SDL_Event event) {
  
  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = true; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = true; }
    
    /* Up key used to "jump". Just adds to up velocity and flaps wings */
    if (event.key.keysym.sym == SDLK_UP) {
      character* main_char = entity_get("main_char");
      main_char->velocity.y -= 5.0;
      main_char->flap_timer = 0.15;
    }
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = false; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = false; }
    
    if (event.key.keysym.sym == SDLK_p) {
      character* main_char = entity_get("main_char");
      debug("Player Position (%0.2f, %0.2f)", main_char->position.x / 32, main_char->position.y / 32);
    }
    
    /* "a" key used to disable and enable audio */
    if (event.key.keysym.sym == SDLK_a) {
      ui_text* audio_text = ui_elem_get("audio_text");
      if(audio_mixer_enabled()) {
        audio_mixer_disable(); debug("Audio Disabled");
        ui_text_update_string(audio_text, "Audio Disabled");
      } else {
        audio_mixer_enable(); debug("Audio Enabled");
        ui_text_update_string(audio_text, "Audio Enabled");
      }
    }
    
  break;
  }
  
  /* Also pass through to the UI for events */
  ui_event(event);
    
}

static void collision_detection() {
  
  /*
    Collision is fairly simplistic and looks something like this.
    
     @-----@    We check for collision in those points here which
    @       @   are crosses. If any are colliding with a solid tile
    |       |   then we shift the player so that they are no longer
    @       @   colliding with it. Also invert their velocity.
     @-----@ 
  
  */
  
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
  
  /* We simply check if the player intersects with the coins */
  
  character* main_char = entity_get("main_char");
  vector2 top_left = v2_add(main_char->position, v2(-32, -32));
  vector2 bottom_right = v2_add(main_char->position, v2(32, 32));
  
  int num_coins = 0;
  coin* coins[COIN_COUNT];
  entities_get(coins, &num_coins, coin); 
  
  int i;
  for(i = 0; i < num_coins; i++) {
    if ((coins[i]->position.x > top_left.x) &&
        (coins[i]->position.x < bottom_right.x) &&
        (coins[i]->position.y > top_left.y) && 
        (coins[i]->position.y < bottom_right.y)) {
      
      char* coin_name = entity_name(coins[i]);
      debug("Got Coin %s!", coin_name);
      audio_mixer_play_sound(asset_get_as("./sounds/coin.wav", sound));
      entity_delete(coin_name);
      
      ui_text* score_text = ui_elem_get("score_text");
      
      level_score += 10;
      sprintf(score_text->string, "Score %06i", level_score);
      ui_text_update_properties(score_text);
    }
  }
  
}

void platformer_update() {
  
  character* main_char = entity_get("main_char");
  
  /* If player has a key held accelerate in that direction */
  const float speed = 0.1;
  
  if (left_held) {
    main_char->velocity.x -= speed;
    main_char->facing_left = true;
  } else if (right_held) {
    main_char->velocity.x += speed;
    main_char->facing_left = false;
  } else {
    main_char->velocity.x *= 0.95;
  }
  
  /* Give the player some gravity speed */
  const float gravity = 0.2;
  main_char->velocity.y += gravity;
  
  /* Update moves position based on velocity */
  character_update(main_char);
  
  /* Two phases of collision detection */
  collision_detection();
  collision_detection_coins();
  
  /* Camera follows main character */
  camera_position = v2(main_char->position.x, -main_char->position.y);
  
  /* Update the framerate text */
  ui_text* framerate_text = ui_elem_get("framerate_text");
  ui_text_update_string(framerate_text, frame_rate_string());
  
  /* Update the time text */
  level_time += frame_time();
  ui_text* time_text = ui_elem_get("time_text");
  sprintf(time_text->string, "Time %06i", (int)level_time);
  ui_text_update_properties(time_text);
  
  /* Update rest of UI */
  ui_update();
  
}

void platformer_render() {
  
  /* Clear the screen to a single color */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  level_render_background(current_level);
  
  /* Render main character */
  character_render(entity_get_as("main_char", character), camera_position);
  
  coin* coins[COIN_COUNT];
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
  
  viewport_set_vsync(true);
  
  platformer_init();
  
  /* Set the game running, create SDL_Event struct to monitor events */
  bool running = 1;
  SDL_Event event;
  
  while(running) {
    
    /* Frame functions used to monitor frame times, FPS and other */
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
    
      switch(event.type){
      case SDL_KEYUP:
        /* Exit on ESCAPE and Screenshot on print screen */
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = false; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        if (event.key.keysym.sym == SDLK_KP_PLUS) {
          audio_mixer_set_volume(audio_mixer_get_volume() + 0.1);
          debug("Volume: %0.2f", audio_mixer_get_volume());
        }
        if (event.key.keysym.sym == SDLK_KP_MINUS) { 
          audio_mixer_set_volume(audio_mixer_get_volume() - 0.1);
          debug("Volume: %0.2f", audio_mixer_get_volume());
        }
        break;
      case SDL_QUIT:
        /* Corrisponds to SDL_QUIT event such as cross in top right corner */
        running = false;
        break;
      }
      
      /* Also send the event off for platformer specific events */
      platformer_event(event);
    }
    
    platformer_update();
    platformer_render();
    
    /* This allows us to fix the framerate to 60 fps, even on my laptop with vsync broken */
    frame_end_at_rate(60);
  }
  
  platformer_finish();
  
  /* Finish Corange. This will unload and assets and delete any remaining entities */
  corange_finish();
  
  return 0;
}
