#include <stdio.h>
#include <math.h>

#include "corange.h"

#include "level.h"
#include "character.h"
#include "coin.h"

#include "platformer.h"

/* Some game state variables */
static level* current_level = NULL;
static vector2 camera_position;
static int level_score = 0;
static float level_time = 0.0;

/* We store all the coin positions here */
#define COIN_COUNT 45
static vector2 coin_positions[COIN_COUNT] = {
  {{16, 23}}, {{33, 28}}, {{41, 22}}, {{20, 19}}, {{18, 28}},
  {{36, 20}}, {{20, 30}}, {{31, 18}}, {{45, 23}}, {{49, 26}},
  {{25, 18}}, {{20, 37}}, {{44, 32}}, {{66, 20}}, {{52, 20}},
  {{63, 11}}, {{52, 12}}, {{39, 13}}, {{27, 11}}, {{73, 20}},
  {{65, 29}}, {{72, 29}}, {{78, 30}}, {{78, 20}}, {{83, 22}},
  {{87, 22}}, {{90, 24}}, {{94, 19}}, {{99, 18}}, {{82, 13}},
  {{79, 14}}, {{106, 22}}, {{102, 30}}, {{100, 35}}, {{93, 27}},
  {{88, 34}}, {{98, 40}}, {{96, 40}}, {{94, 40}}, {{86, 40}},
  {{81, 37}}, {{77, 38}}, {{72, 34}}, {{65, 38}}, {{71, 37}}
};

static void reset_game() {

  /* Set the starting level to demo.level */
  current_level = asset_get("./levels/demo.level");
  level_score = 0;
  level_time = 0.0;
  
  /* New main character entity */
  character* main_char = entity_get("main_char");
  main_char->position = v2_mul( v2(20, 20), TILE_SIZE);
  main_char->velocity = v2_zero();
  
  /* We can create multiple entities using a name format string like printf */
  entities_new("coin_id_%i", COIN_COUNT, coin);
  
  /* Get an array of pointers to all coin entities */
  coin* coins[COIN_COUNT];
  entities_get(coins, NULL, coin);
  
  /* Set all the coin initial positions */
  for(int i = 0; i < COIN_COUNT; i++) {
    coins[i]->position = v2_mul(coin_positions[i], TILE_SIZE);
  }
  
  /* Deactivate victory and new game UI elements */
  ui_rectangle* victory_rect = ui_elem_get("victory_rect");
  ui_text* victory_text = ui_elem_get("victory_text");
  ui_rectangle* new_game_rect = ui_elem_get("new_game_rect");
  ui_text* new_game_text = ui_elem_get("new_game_text");
  
  victory_rect->active = false;
  victory_text->active = false;
  new_game_rect->active = false;
  new_game_text->active = false;
  
}

/* This is an event we attach to the audio button */
static bool disable_audio_pressed = false;
static void disable_audio(ui_rectangle* rect, SDL_Event event) {
  
  /* On click down the button changes color */
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      disable_audio_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  /* On click up it disables/enabled the audio and changes back */
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (disable_audio_pressed) {
      disable_audio_pressed = false;
      rect->color = v4_black();
      
      if (audio_mixer_enabled()) {
        audio_mixer_disable();
        ui_text* audio_text = ui_elem_get("audio_text");
        ui_text_update_string(audio_text, "Enable Audio");
      } else {
        audio_mixer_enable();
        ui_text* audio_text = ui_elem_get("audio_text");
        ui_text_update_string(audio_text, "Disable Audio");
      }
      
    }
  }
}

/* This is an event we attach to the new game button */
static bool new_game_pressed = false;
static void new_game(ui_rectangle* rect, SDL_Event event) {
  
  /* On click down the button changes color */
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    
    if (ui_rectangle_contains_position(rect, v2(event.motion.x, event.motion.y))) {
      new_game_pressed = true;
      rect->color = v4(0.5, 0.5, 0.5, 1);
    }
  
  /* On click up it resets the game */
  } else if (event.type == SDL_MOUSEBUTTONUP) {
    
    if (new_game_pressed) {
      new_game_pressed = false;
      rect->color = v4_black();
      reset_game();
    }
  }
}

void platformer_init() {
  
  /* Register functions for loading/unloading files with the extension .level */
  asset_manager_handler("level", level_load_file, level_delete);
  
  /* Load Assets */
  load_folder("./tiles/");
  load_folder("./backgrounds/");
  load_folder("./sounds/");
  load_folder("./levels/");
  
  /* Register some handlers for creating and destroying entity types */
  entity_manager_handler(character, character_new, character_delete);
  entity_manager_handler(coin, coin_new, coin_delete);
  
  /* Create out main character */
  character* main_char = entity_new("main_char", character);
  
  /* Add some UI elements */
  ui_rectangle* framerate_rect = ui_elem_new("framerate_rect", ui_rectangle);
  framerate_rect->top_left = v2(10, 10);
  framerate_rect->bottom_right = v2(40, 35);
  framerate_rect->color = v4_black();
  framerate_rect->border_color = v4_white();
  framerate_rect->border_size = 1;
  
  ui_text* framerate_text = ui_elem_new("framerate_text", ui_text);
  framerate_text->position = v2(17, 15);
  framerate_text->color = v4_white();
  ui_text_update_string(framerate_text, "framerate");
  
  ui_rectangle* audio_rect = ui_elem_new("audio_rect", ui_rectangle);
  audio_rect->top_left = v2(50, 10);
  audio_rect->bottom_right = v2(170, 35);
  audio_rect->color = v4_black();
  audio_rect->border_color = v4_white();
  audio_rect->border_size = 1;
  
  ui_elem_add_event("audio_rect", disable_audio);
  
  ui_text* audio_text = ui_elem_new("audio_text", ui_text);
  audio_text->position = v2(60, 15);
  audio_text->color = v4_white();
  ui_text_update_string(audio_text, "Disable Audio");
  
  ui_rectangle* score_rect = ui_elem_new("score_rect", ui_rectangle);
  score_rect->top_left = v2(180, 10);
  score_rect->bottom_right = v2(300, 35);
  score_rect->color = v4_black();
  score_rect->border_color = v4_white();
  score_rect->border_size = 1;
  
  ui_text* score_text = ui_elem_new("score_text", ui_text);
  score_text->position = v2(190, 15);
  score_text->color = v4_white();
  ui_text_update_string(score_text, "Score 000000");
  
  ui_rectangle* time_rect = ui_elem_new("time_rect", ui_rectangle);
  time_rect->top_left = v2(310, 10);
  time_rect->bottom_right = v2(420, 35);
  time_rect->color = v4_black();
  time_rect->border_color = v4_white();
  time_rect->border_size = 1;
  
  ui_text* time_text = ui_elem_new("time_text", ui_text);
  time_text->position = v2(320, 15);
  time_text->color = v4_white();
  ui_text_update_string(time_text, "Time 000000");
  
  ui_rectangle* victory_rect = ui_elem_new("victory_rect", ui_rectangle);
  victory_rect->top_left = v2(365, 200);
  victory_rect->bottom_right = v2(435, 225);
  victory_rect->color = v4_black();
  victory_rect->border_color = v4_white();
  victory_rect->border_size = 1;
  victory_rect->active = false;
  
  ui_text* victory_text = ui_elem_new("victory_text", ui_text);
  victory_text->position = v2(370, 205);
  victory_text->color = v4_white();
  victory_text->active = false;
  ui_text_update_string(victory_text, "Victory!");
  
  ui_rectangle* new_game_rect = ui_elem_new("new_game_rect", ui_rectangle);
  new_game_rect->top_left = v2(365, 230);
  new_game_rect->bottom_right = v2(435, 255);
  new_game_rect->color = v4_black();
  new_game_rect->border_color = v4_white();
  new_game_rect->border_size = 1;
  new_game_rect->active = false;
  
  ui_text* new_game_text = ui_elem_new("new_game_text", ui_text);
  new_game_text->position = v2(370, 235);
  new_game_text->color = v4_white();
  new_game_text->active = false;
  ui_text_update_string(new_game_text, "New Game");
  
  ui_elem_add_event("new_game_rect", new_game);
  
  /* Set volume to something more reasonable */
  audio_mixer_set_volume(0.1);
  
  /* Reset all the game variables */
  reset_game();
  
}

/* Some booleans to monitor key inputs */
static bool left_held = false;
static bool right_held = false;

void platformer_event(SDL_Event event) {
  
  switch(event.type){
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = true; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = true; }
    
    /* Up key used to "jump". Just adds to up velocity and flaps wings of icon */
    if (event.key.keysym.sym == SDLK_UP) {
      character* main_char = entity_get("main_char");
      main_char->velocity.y -= 5.0;
      main_char->flap_timer = 0.15;
    }
    
  break;
  
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_LEFT) { left_held = false; }
    if (event.key.keysym.sym == SDLK_RIGHT) { right_held = false; }
  break;
  }
    
}

static void collision_detection() {
  
  /*
    Collision is fairly simplistic and looks something like this.
    
     @-----@    We check for collision in those points here which
    @       @   are @ signs. If any are colliding with a solid tile
    |       |   then we shift the player so that they are no longer
    @       @   colliding with it. Also invert the velocity.
     @-----@ 
  */
  
  character* main_char = entity_get("main_char");
  
  const float buffer = 4;
  const float bounce = 0.5;
  
  vector2 diff;
  
  /* Bottom Collision */
  
  diff = v2_fmod(main_char->position, TILE_SIZE);
  
  vector2 bottom1 = v2_add(main_char->position, v2(buffer, TILE_SIZE));
  vector2 bottom2 = v2_add(main_char->position, v2(TILE_SIZE - buffer, TILE_SIZE));
  
  bool bottom1_col = tile_has_collision(level_tile_at(current_level, bottom1));
  bool bottom2_col = tile_has_collision(level_tile_at(current_level, bottom2));
  
  if (bottom1_col || bottom2_col) {
    main_char->position = v2_add(main_char->position, v2(0,-diff.y));
    main_char->velocity.y *= -bounce;
  }
  
  /* Top Collision */
  
  diff = v2_fmod(main_char->position, TILE_SIZE);
  
  vector2 top1 = v2_add(main_char->position, v2(buffer, 0));
  vector2 top2 = v2_add(main_char->position, v2(TILE_SIZE - buffer, 0));
  
  bool top1_col = tile_has_collision(level_tile_at(current_level, top1));
  bool top2_col = tile_has_collision(level_tile_at(current_level, top2));
  
  if (top1_col || top2_col) {
    main_char->position = v2_add(main_char->position, v2(0, TILE_SIZE - diff.y));
    main_char->velocity.y *= -bounce;
  }
  
  /* Left Collision */
  
  diff = v2_fmod(main_char->position, TILE_SIZE);
  
  vector2 left1 = v2_add(main_char->position, v2(0, buffer));
  vector2 left2 = v2_add(main_char->position, v2(0, TILE_SIZE - buffer));
  
  bool left1_col = tile_has_collision(level_tile_at(current_level, left1));
  bool left2_col = tile_has_collision(level_tile_at(current_level, left2));
  
  if (left1_col || left2_col) {
    main_char->position = v2_add(main_char->position, v2(TILE_SIZE - diff.x,0));
    main_char->velocity.x *= -bounce;
  }
  
  /* Right Collision */
  
  diff = v2_fmod(main_char->position, TILE_SIZE);
  
  vector2 right1 = v2_add(main_char->position, v2(TILE_SIZE, buffer));
  vector2 right2 = v2_add(main_char->position, v2(TILE_SIZE, TILE_SIZE - buffer));
  
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
  
  vector2 top_left = v2_add(main_char->position, v2(-TILE_SIZE, -TILE_SIZE));
  vector2 bottom_right = v2_add(main_char->position, v2(TILE_SIZE, TILE_SIZE));
  
  /* Again we collect pointers to all the coin type entities */
  int num_coins = 0;
  coin* coins[COIN_COUNT];
  entities_get(coins, &num_coins, coin); 
  
  for(int i = 0; i < num_coins; i++) {
    /* Check if they are within the main char bounding box */
    if ((coins[i]->position.x > top_left.x) &&
        (coins[i]->position.x < bottom_right.x) &&
        (coins[i]->position.y > top_left.y) && 
        (coins[i]->position.y < bottom_right.y)) {
      
      /* Remove them from the entity manager and delete */
      char* coin_name = entity_name(coins[i]);
      entity_delete(coin_name);
      
      /* Play a nice twinkle sound */
      audio_mixer_play_sound(asset_get_as("./sounds/coin.wav", sound));
      
      /* Add some score! */
      level_score += 10;
      
      /* Update the ui text */
      ui_text* score_text = ui_elem_get("score_text");
      sprintf(score_text->string, "Score %06i", level_score);
      ui_text_update_properties(score_text);
    }
  }
  
  ui_rectangle* victory_rect = ui_elem_get("victory_rect");
  
  /* if all the coins are gone and the victory rectangle isn't disaplayed then show it */
  if ((entity_type_count(coin) == 0) && (!victory_rect->active)) {
    ui_rectangle* victory_rect = ui_elem_get("victory_rect");
    victory_rect->active = true;
    ui_text* victory_text = ui_elem_get("victory_text");
    victory_text->active = true;
    ui_rectangle* new_game_rect = ui_elem_get("new_game_rect");
    new_game_rect->active = true;
    ui_text* new_game_text = ui_elem_get("new_game_text");
    new_game_text->active = true;
  }
  
}

void platformer_update() {
  
  character* main_char = entity_get("main_char");
  
  if (left_held) {
    main_char->velocity.x -= 0.1;
    main_char->facing_left = true;
  } else if (right_held) {
    main_char->velocity.x += 0.1;
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
  ui_rectangle* victory_rect = ui_elem_get("victory_rect");
  if (!victory_rect->active) {
    level_time += frame_time();
    ui_text* time_text = ui_elem_get("time_text");
    sprintf(time_text->string, "Time %06i", (int)level_time);
    ui_text_update_properties(time_text);
  }
  
}

void platformer_render() {
  
  /* Clear the screen to a single color */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  level_render_background(current_level);
  
  character_render(entity_get_as("main_char", character), camera_position);
  
  /* Get pointers to all the coins for rendering */
  coin* coins[COIN_COUNT];
  int num_coins = 0;
  entities_get(coins, &num_coins, coin); 
  
  for(int i = 0; i < num_coins; i++) {
    coin_render(coins[i], camera_position);
  }
  
  level_render_tiles(current_level, camera_position);
}

void platformer_finish() {
  
  /* Entity and asset managers will automatically delete any remaining objects. */
  
}

int main(int argc, char **argv) {
  
  /* Init Corange, pointing to the core_assets folder */
  corange_init("../../core_assets");
  
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
        break;
      case SDL_QUIT:
        /* A quitting event such as pressing cross in top right corner */
        running = false;
        break;
      }
      
      /* Also send this event off to the game and ui */
      platformer_event(event);
      ui_event(event);
    }
    
    platformer_update();
    ui_update();
    
    platformer_render();
    ui_render();
    
    /* Flip the Screen Buffer. We've finished with this frame. */
    SDL_GL_SwapBuffers(); 
    
    /* This allows us to fix the framerate to 60 fps, even on my laptop with vsync broken */
    frame_end_at_rate(60);
  }
  
  platformer_finish();
  
  /* Corange will unload remaining assets and delete any remaining entities */
  corange_finish();
  
  return 0;
}
