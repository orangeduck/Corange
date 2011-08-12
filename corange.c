#include <stdlib.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"

#include "timing.h"
#include "font.h"
#include "texture.h"
#include "camera.h"
#include "vector.h"
#include "geometry.h"
#include "material.h"
#include "scripting.h"
#include "shader.h"

#include "perlin_noise.h"

#include "asset_manager.h"
#include "obj_loader.h"
#include "ply_loader.h"

#include "game.h"

static char* game_name_arg;

main(int argc, char* argv[]) {
  
  /* Stop Redirect of stdout and stderr */
  
  FILE * ctt = fopen("CON", "w" );
  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
  
  /* SDL Setup */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  /* Init SDL image */
  
  int flags = IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF;
  int initted = IMG_Init(flags);
  if(initted&flags != flags) {
      printf("IMG_Init: Failed to init required jpg and png support!\n",0);
      printf("IMG_Init: %s\n", IMG_GetError());
  }
  
  viewport_init();
  
  /* OpenGL setup */
  
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    printf("Glew Error: %s\n", glewGetErrorString(err));
  }
  
  /* Start */
    
  if (argc < 2) {
    game_name_arg = malloc(strlen("empty") + 1);
    strcpy(game_name_arg, "empty");
  } else {
    game_name_arg = malloc(strlen(argv[1]) + 1);
    strcpy(game_name_arg, argv[1]);
  }
  
  /* Load Assets */
  
  asset_manager_init(game_name_arg);
  
  asset_manager_handler("obj", (void*(*)(char*))obj_load_file, (void(*)(void*))model_delete);
  asset_manager_handler("ply", (void*(*)(char*))ply_load_file, (void(*)(void*))model_delete);
  
  asset_manager_handler("dds", (void*(*)(char*))dds_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("bmp", (void*(*)(char*))bmp_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("png", (void*(*)(char*))png_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("tif", (void*(*)(char*))tif_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("jpg", (void*(*)(char*))jpg_load_file, (void(*)(void*))texture_delete);
  
  asset_manager_handler("vs" , (void*(*)(char*))vs_load_file,  (void(*)(void*))shader_delete);
  asset_manager_handler("fs" , (void*(*)(char*))fs_load_file,  (void(*)(void*))shader_delete);
  asset_manager_handler("prog",(void*(*)(char*))prog_load_file,(void(*)(void*))shader_program_delete);
  
  asset_manager_handler("fnt", (void*(*)(char*))font_load_file,(void(*)(void*))font_delete);
  asset_manager_handler("mat", (void*(*)(char*))mat_load_file, (void(*)(void*))material_delete);
  asset_manager_handler("lua", (void*(*)(char*))lua_load_file, (void(*)(void*))script_delete);
  
  load_folder("./engine/shaders/");
  load_folder("./engine/fonts/");
  load_folder("./engine/scripts/");

  load_folder("./engine/resources/");
  
  /* Setup Scripting */
  
  scripting_init();
  
  /* Load Game */
  
  game_load(game_name_arg);
  
  game_init();
  
  SDL_Event event;
  int running = 1;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      game_event(event);  
    }
    
    game_update();
    
    game_render();
    
    glFlush();
    SDL_GL_SwapBuffers();   
    
    frame_end();
  }
  
  game_finish();
  
  game_unload();
  
  /* Unload assets */
  
  asset_manager_finish();
  
  scripting_finish();
  
  viewport_finish();
  
  IMG_Quit();
  SDL_Quit();
  
  return 0;
}
