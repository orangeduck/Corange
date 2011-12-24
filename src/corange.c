#include <stdlib.h>

#include "corange.h"

void corange_stop_stdout_redirect() {
  FILE* ctt = fopen("CON", "w" );
  FILE* fout = freopen( "CON", "w", stdout );
  FILE* ferr = freopen( "CON", "w", stderr );
}

void corange_init(char* core_assets_path) {
  
  printf("Starting Corange...\n");
  
  /* Stop Redirect of stdout and stderr */
  
  corange_stop_stdout_redirect();
  
  /* Load asset path stuff */
  
  asset_manager_add_path_variable("$CORANGE", core_assets_path);
  
  /* SDL Setup */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    error("Unable to initialize SDL: %s\n", SDL_GetError());
  }
  
  viewport_init();
  
  SDL_LoadOpenGLExtensions();
  
  /* Load Assets */
  
  asset_manager_init();
  
  asset_manager_handler("obj", obj_load_file, renderable_delete);
  asset_manager_handler("smd", smd_load_file, renderable_delete);
  asset_manager_handler("skl", skl_load_file, skeleton_delete);
  asset_manager_handler("ani", ani_load_file, animation_delete);
  
  asset_manager_handler("bmp", bmp_load_file, image_delete);
  asset_manager_handler("tga", tga_load_file, image_delete);
  asset_manager_handler("dds", dds_load_file, texture_delete);
  asset_manager_handler("lut", lut_load_file, texture_delete);
  
  asset_manager_handler("vs" , vs_load_file,  shader_delete);
  asset_manager_handler("fs" , fs_load_file,  shader_delete);
  asset_manager_handler("prog",prog_load_file,shader_program_delete);
  
  asset_manager_handler("fnt", font_load_file,font_delete);
  asset_manager_handler("mat", mat_load_file, material_delete);
  asset_manager_handler("lua", lua_load_file, script_delete);
  
  load_folder("$CORANGE/shaders/");
  load_folder("$CORANGE/fonts/");
  load_folder("$CORANGE/scripts/");

  load_folder("$CORANGE/resources/");
  
  /* Setup Scripting */
  
  scripting_init();
  
  /* Entity Manager */
  
  entity_manager_init();
  
  printf("Done!\n");
  
}

void corange_finish() {

  /* Unload assets */
  
  entity_manager_finish();
  
  asset_manager_finish();
  
  scripting_finish();
  
  viewport_finish();
  
  SDL_Quit();

}
