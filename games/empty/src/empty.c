#include "corange.h"

static font* console_font;
static ui_text* no_game;

void empty_init() {
  
  console_font = asset_get("./engine/fonts/console_font.fnt");

  no_game = ui_text_new("No Game Loaded.", console_font);
  no_game->scale = v2(3.0, 3.0);
  no_game->position = v2(viewport_width() / 2,viewport_height() / 2);
  no_game->color = v4(1.0,1.0,1.0,1);
  no_game->alignment = text_align_center;
  no_game->vertical_alignment = text_align_center;
  ui_text_update(no_game);
  
}

void empty_update() {

}

void empty_render() {

  ui_text_render(no_game);

}

void empty_event(SDL_Event event) {

}

void empty_finish() {

  ui_text_delete(no_game);

}