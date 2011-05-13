#include <stdlib.h>
#include <time.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

#include "font.h"
#include "texture.h"
#include "camera.h"
#include "vector.h"
#include "renderer.h"
#include "geometry.h"

#include "obj_loader.h"

#include "asset_manager.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

void DisplayState(SDL_KeyboardEvent *key) {
  if (key->type == SDL_KEYUP)
    printf("RELEASED: ");
  else
    printf("PRESSED: ");
}

void DisplayModifiers(SDL_KeyboardEvent *key) {
  SDLMod modifier = key->keysym.mod;
  if( modifier & KMOD_NUM ) printf( "NUMLOCK " );
  if( modifier & KMOD_CAPS ) printf( "CAPSLOCK " );
  if( modifier & KMOD_MODE ) printf( "MODE " );
  if( modifier & KMOD_LCTRL ) printf( "LCTRL " );
  if( modifier & KMOD_RCTRL ) printf( "RCTRL " );
  if( modifier & KMOD_LSHIFT ) printf( "LSHIFT " );
  if( modifier & KMOD_RSHIFT ) printf( "RSHIFT " );
  if( modifier & KMOD_LALT ) printf( "LALT " );
  if( modifier & KMOD_RALT ) printf( "RALT " );
  if( modifier & KMOD_LMETA ) printf( "LMETA " );
  if( modifier & KMOD_RMETA ) printf( "RMETA " );
}

void DisplayKey(SDL_KeyboardEvent *key) {
  printf( "%s\n", SDL_GetKeyName(key->keysym.sym));
}

main(int argc, char *argv[]) {
  
  SDL_Surface *screen;
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  int flags = IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF;
  int initted = IMG_Init(flags);
  if(initted&flags != flags) {
      printf("IMG_Init: Failed to init required jpg and png support!\n");
      printf("IMG_Init: %s\n", IMG_GetError());
  }
  
  /* Set Window properties */
  
  SDL_WM_SetCaption("Corange","Corange");    
  SDL_Surface* image = IMG_Load("icon.png");
  SDL_WM_SetIcon(image, NULL);
  
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 16 );
  
  screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 0, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
  if (screen == NULL) {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }
  
  /* openGL setup */
  
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    printf("Glew Error: %s\n", glewGetErrorString(err));
  }
  
  renderer_setup();
  renderer_set_dimensions(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  
  /* End openGL setup */
  
  /* Load Assets */
  
  
  asset_manager_init();
  
  asset_manager_handler("obj", (void*(*)(char*))obj_load_file, (void(*)(void*))model_delete);
  asset_manager_handler("dds", (void*(*)(char*))dds_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("bmp", (void*(*)(char*))bmp_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("png", (void*(*)(char*))png_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("tif", (void*(*)(char*))tif_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("jpg", (void*(*)(char*))jpg_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("fnt", (void*(*)(char*))font_load_file, (void(*)(void*))font_delete);
  
  load_folder("./Engine/Assets/Textures/");
  load_folder("./Engine/Assets/Meshes/");
  load_folder("./Engine/Assets/Fonts/");
  
  camera* cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  renderer_set_camera(cam);
  
  render_model* piano = (render_model*)asset_get("./Engine/Assets/Meshes/piano.obj");
  font* console_font = (font*)asset_get("./Engine/Assets/Fonts/console_font.fnt");
  
  /* Setup Framerate variables */
  
  long start_time, end_time;
  float acc_time;
  
  int frame_count = 0;
  int frame_rate = 0;
  char frame_rate_s[10] = "";

  float frame_update_rate = 0.5;
  
  /* Start */
  
  SDL_Event event;
  int running = 1;
  
  while(running) {
  
  start_time = clock();
  
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        
        if (event.key.keysym.sym == SDLK_UP) { cam->position.x -= 1.0;}
        if (event.key.keysym.sym == SDLK_DOWN) { cam->position.x += 1.0;}
        
        if (event.key.keysym.sym == SDLK_LEFT) { cam->position.z -= 1.0;}
        if (event.key.keysym.sym == SDLK_RIGHT) { cam->position.z += 1.0;}

        if (event.key.keysym.sym == SDLK_LEFTBRACKET) { cam->position.y += 1.0;}
        if (event.key.keysym.sym == SDLK_RIGHTBRACKET) { cam->position.y -= 1.0;}
        
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
    }
    
    /* Begin Rendering */
    renderer_begin_render();
    
    renderer_render_model(piano);
    renderer_render_string(frame_rate_s, console_font, v2(-1.0,-1.0) , 1.25);
   
    renderer_end_render();
    /* End Rendering */
    
    /* Calculate Framerate */
    
    frame_count++;
    
    end_time = clock();
    acc_time = acc_time + ((float)(end_time - start_time) / (float)CLOCKS_PER_SEC);
    
    if (acc_time > frame_update_rate) {  
      frame_rate = (int)(frame_count / frame_update_rate);
      itoa(frame_rate, frame_rate_s, 10);
      frame_count = 0;
      acc_time = 0.0;  
    }
  
  }	
  
  /* Finish */
  
  renderer_finish();
  
  /* Unload assets */
  
  asset_manager_finish();
  
  camera_delete(cam);
  
  IMG_Quit();
  SDL_Quit();
  
  return 0;
}
