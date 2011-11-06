#include "corange.h"

#include "spline.h"

spline* test_spline;

int mouse_down = 0;

void splines_init() {
  
  asset_manager_handler("acv", (void*(*)(char*))acv_load_file, (void(*)(void*))color_curves_delete);
  
  load_folder("/resources/");
  
  color_curves_write_lut(asset_get("/resources/test.acv"), "./funky.lut");
  color_curves_write_lut(asset_get("/resources/test2.acv"), "./identity.lut");
  color_curves_write_lut(asset_get("/resources/bluey.acv"), "./bluey.lut");
  
  test_spline = spline_new();
  spline_add_point(test_spline, v2(0,0));
  spline_add_point(test_spline, v2(0.5,0.5));
  spline_add_point(test_spline, v2(1,1));
  spline_update(test_spline);
}

void splines_update() {

}

void splines_render() {
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  spline_render(test_spline, v2(10, 10), v2(400, 400), 50);
}

void splines_event(SDL_Event event) {
  
  switch(event.type){

  case SDL_KEYUP:
    
  break;
  
  case SDL_MOUSEBUTTONUP:
    mouse_down = 0;
  break;
  
  case SDL_MOUSEBUTTONDOWN:
    mouse_down = 1;
  break;
  
  case SDL_MOUSEMOTION:
    if(mouse_down) {
      float x = (float)(event.button.x - 10) / 400;
      float y = (float)(event.button.y - 10) / 400;
      spline_set_point(test_spline, 1, v2(x,y));
      spline_update(test_spline);
    }
  break;
  }
  
}

void splines_finish() {
  spline_delete(test_spline);
}