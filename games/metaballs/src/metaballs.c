#include "corange.h"

static font* console_font;
static render_text* rt_no_game;

void metaballs_init() {
  
  console_font = asset_get("./engine/fonts/console_font.fnt");

  rt_no_game = render_text_new("METABALLS.", 512, console_font);
  rt_no_game->scale = v2(3.0, 3.0);
  rt_no_game->position = v2(-0.65,-0.1);
  rt_no_game->color = v4(1.0,1.0,1.0,1);
  render_text_update(rt_no_game);
  
}

void metaballs_update() {

}

void metaballs_render() {

  render_text_render(rt_no_game);

}

void metaballs_event(SDL_Event event) {

}

void metaballs_finish() {

  render_text_delete(rt_no_game);

}