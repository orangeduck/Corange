#include <stdlib.h>

#include "corange.h"

static char* game_name_arg;

int main(int argc, char* argv[]) {
  
  /* Stop Redirect of stdout and stderr */
  
  FILE* ctt = fopen("CON", "w" );
  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
  
  /* SDL Setup */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  
  viewport_init();
  
  SDL_LocalInit();
      
  /* OpenCL setup */
  
  /*
  
  cl_int error;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  
  error = oclGetPlatformID(&platform);
  if (error != CL_SUCCESS) {
     printf("Error getting platform id (%i)\n", error);
     exit(EXIT_FAILURE);
  }
  
  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (error != CL_SUCCESS) {
    printf("Error getting OpenCL device (%i)\n", error);
    exit(EXIT_FAILURE);
  }
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  if (error != CL_SUCCESS) {
    printf("Error creating OpenCL context (%i)\n", error);
    exit(EXIT_FAILURE);
  }
  
  kernel_set_device(device);
  kernel_set_context(context);
  
  */
  
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
  
  asset_manager_handler("dds", (void*(*)(char*))dds_load_file, (void(*)(void*))texture_delete);
  
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
  
  SDL_Quit();
  
  return 0;
}
