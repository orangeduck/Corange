#include <stdio.h>

#include "corange.h"

#include "wolf_renderer.h"
#include "wolf_renderable.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

static model* piano;
static material* piano_mat;
static renderable* r_piano;
static wolf_renderable* wr_piano;

static font* console_font;
static render_text* rt_framerate;
static render_text* rt_test_text;

static camera* cam;
static light* sun;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

void wolf_init() {
  
  printf("Wolf game init!\n");
  
  /* Get reference to the Cello */
  
  load_folder("/resources/");
  load_folder("/resources/piano/");
  load_folder("/resources/shaders/");
  
  texture* brush = asset_get("/resources/brush_dict.dds");
  
  piano = asset_get("/resources/piano/piano.obj");
  piano_mat = asset_get("/resources/piano/piano.mat");

  r_piano = renderable_new("piano");
  renderable_add_model(r_piano, piano);
  renderable_set_material(r_piano, piano_mat);
  
  wr_piano = wolf_renderable_new("wr_piano", piano, brush, 8, v2(1,1), 800.0 );
  renderable_set_material(wr_piano->renderable, piano_mat);
  
  /* Put some text on the screen */
  
  console_font = asset_get("./engine/fonts/console_font.fnt");
  
  rt_framerate = render_text_new("hello", 10, console_font);
  rt_framerate->position = v2(-0.95,-0.95);
  rt_framerate->scale = v2(0.7,0.7);
  rt_framerate->color = v4(1,1,1,1);
  render_text_update(rt_framerate);
  
  rt_test_text = render_text_new("Painting Renderer\nmouse to move\n'p' to switch object.", 512, console_font);
  rt_test_text->position = v2(-0.95,-0.90);
  rt_test_text->scale = v2(0.7,0.7);
  rt_test_text->color = v4(1,1,1,1);
  render_text_update(rt_test_text);
  
  /* Init render engine */
  
  viewport_set_vsync(1);
  //viewport_set_dimensions( v2(800 * 1.5, 600 * 1.5) );
  
  /* New Camera */
  
  cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  sun = light_new_type( v3(30,43,-26), light_type_spot );
  
  sun->ambient_color = v3(0.749, 0.855, 0.902);
  sun->diffuse_color = v3(1.0, 0.875, 0.573);
  
  /* Renderer Setup */

  shadow_mapper_init(sun);  
  
  printf("Starting Renderer\n");fflush(stdout);
  
  wolf_renderer_init();
  wolf_renderer_set_camera(cam);
  wolf_renderer_set_light(sun);
  wolf_renderer_set_shadow_texture( shadow_mapper_depth_texture() );
    
}

void wolf_update() {

  Uint8 keystate = SDL_GetMouseState(NULL, NULL);
  if(keystate & SDL_BUTTON(1)){
    float a1 = -(float)mouse_x * 0.0025;
    float a2 = (float)mouse_y * 0.0025;
    
    cam->position = m33_mul_v3(m33_rotation_y( a1 ), cam->position );
    
    vector3 rotation_axis = v3_normalize(v3_cross( v3_sub(cam->position, cam->target) , v3(0,1,0) ));
    
    cam->position = m33_mul_v3(m33_rotation_axis_angle(rotation_axis, a2 ), cam->position );
  }
  
  if(keystate & SDL_BUTTON(3)){
    sun->position.x += (float)mouse_y / 2;
    sun->position.z -= (float)mouse_x / 2;
  }

  mouse_x = 0;
  mouse_y = 0;
  
}

void wolf_event(SDL_Event event) {

  switch(event.type){
  case SDL_KEYUP:
        
  break;

  case SDL_MOUSEBUTTONDOWN:

    if (event.button.button == SDL_BUTTON_WHEELUP) {
      cam->position = v3_sub(cam->position, v3_normalize(cam->position));
    }
    if (event.button.button == SDL_BUTTON_WHEELDOWN) {
      cam->position = v3_add(cam->position, v3_normalize(cam->position));
    }
    
  break;
  
  case SDL_MOUSEMOTION:
    mouse_x = event.motion.xrel;
    mouse_y = event.motion.yrel;
  break;
  }
    
}

void wolf_render() {

  shadow_mapper_begin();
  shadow_mapper_render_renderable(r_piano);
  shadow_mapper_end();

  wolf_renderer_begin();
  wolf_renderer_draw(wr_piano);
  wolf_renderer_end();
  
  wolf_renderer_begin_painting();
  wolf_renderer_paint(wr_piano);
  wolf_renderer_end_painting();

  /* Render text */
  
  //render_text_update_string(rt_framerate, frame_rate_string());
  //render_text_update_string(rt_framerate, "hello");
  
  //render_text_render(rt_framerate);
  //render_text_render(rt_test_text);
  
}

void wolf_finish() {

  shadow_mapper_finish();
  wolf_renderer_finish();
  
  wolf_renderable_delete(wr_piano);
  
  camera_delete(cam);
  light_delete(sun);

  render_text_delete(rt_framerate);
  render_text_delete(rt_test_text);

  printf("Cello game finish!\n");

}