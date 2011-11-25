#include <stdlib.h>

#include "type.h"

#include "corange.h"

static char* game_name_arg;

int main(int argc, char* argv[]) {
  
  /* Stop Redirect of stdout and stderr */
  
  FILE* ctt = fopen("CON", "w" );
  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
  
  /* SDL Setup */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    error("Unable to initialize SDL: %s\n", SDL_GetError());
  }
  
  viewport_init();
  
  SDL_LoadOpenGLExtensions();
      
  /* OpenCL setup */
  
#ifdef _WIN32
  kernels_init_with_opengl();
#else
  kernels_init();
#endif
  
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
  
  asset_manager_handler("obj", (asset_loader_t)obj_load_file, (asset_deleter_t)renderable_delete);
  asset_manager_handler("smd", (asset_loader_t)smd_load_file, (asset_deleter_t)renderable_delete);
  asset_manager_handler("skl", (asset_loader_t)skl_load_file, (asset_deleter_t)skeleton_delete);
  asset_manager_handler("ani", (asset_loader_t)ani_load_file, (asset_deleter_t)animation_delete);
  
  asset_manager_handler("bmp", (asset_loader_t)bmp_load_file, (asset_deleter_t)image_delete);
  asset_manager_handler("tga", (asset_loader_t)tga_load_file, (asset_deleter_t)image_delete);
  asset_manager_handler("dds", (asset_loader_t)dds_load_file, (asset_deleter_t)texture_delete);
  asset_manager_handler("lut", (asset_loader_t)lut_load_file, (asset_deleter_t)texture_delete);
  
  asset_manager_handler("vs" , (asset_loader_t)vs_load_file,  (asset_deleter_t)shader_delete);
  asset_manager_handler("fs" , (asset_loader_t)fs_load_file,  (asset_deleter_t)shader_delete);
  asset_manager_handler("prog",(asset_loader_t)prog_load_file,(asset_deleter_t)shader_program_delete);
  
  asset_manager_handler("fnt", (asset_loader_t)font_load_file,(asset_deleter_t)font_delete);
  asset_manager_handler("mat", (asset_loader_t)mat_load_file, (asset_deleter_t)material_delete);
  asset_manager_handler("lua", (asset_loader_t)lua_load_file, (asset_deleter_t)script_delete);
  asset_manager_handler("cl" , (asset_loader_t)cl_load_file,  (asset_deleter_t)kernel_program_delete);
  
  load_file("./engine/resources/basic.mat");
  
  load_folder("./engine/shaders/");
  load_folder("./engine/fonts/");
  load_folder("./engine/scripts/");
  load_folder("./engine/kernels/");

  load_folder("./engine/resources/");
  
  /* Setup Scripting */
  
  scripting_init();
  
  /* Entity Manager */
  
  entity_manager_init();
  
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
  
  entity_manager_finish();
  
  asset_manager_finish();
  
  scripting_finish();
  
  kernels_finish();
  
  viewport_finish();
  
  SDL_Quit();
  
  return 0;
}
