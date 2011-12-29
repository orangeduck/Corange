#include <stdlib.h>

#include "corange.h"

void corange_init(char* core_assets_path) {
  
  /* Starting Corange */
  
  printf("Starting Corange...\n");
  
  /* Stop stdout redirect on windows */
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  /* Init OpenGL and Viewport */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    error("Unable to initialize SDL: %s\n", SDL_GetError());
  }
  
  viewport_init();
  
  SDL_LoadOpenGLExtensions();
  SDL_PrintOpenGLInfo();
  
  /* Asset Manager */
  
  asset_manager_init();
  
  asset_manager_add_path_variable("$CORANGE", core_assets_path);
  
  char* shaders_path = malloc(strlen(core_assets_path) + strlen("/shaders_nolink") + 1);
  strcpy(shaders_path, core_assets_path);
  
  if ( SDL_OpenGLSupportsShaderLinkage() ) {
    strcat(shaders_path, "/shaders");
    asset_manager_add_path_variable("$SHADERS", shaders_path);
  } else {
    strcat(shaders_path, "/shaders_nolink");
    asset_manager_add_path_variable("$SHADERS", shaders_path);
  }
  
  free(shaders_path);
  
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
  
  /* Entity Manager */
  
  entity_manager_init();
  
  entity_manager_handler(static_object, static_object_new, static_object_delete);
  entity_manager_handler(animated_object, animated_object_new, animated_object_delete);
  entity_manager_handler(camera, camera_new, camera_delete);
  entity_manager_handler(light, light_new, light_delete);
  
  /* UI Manager */
  
  ui_manager_init();
  
  ui_manager_handler(ui_rectangle, ui_rectangle_new, ui_rectangle_delete, ui_rectangle_event, ui_rectangle_update, ui_rectangle_render);
  ui_manager_handler(ui_text, ui_text_new, ui_text_delete, ui_text_event, ui_text_update, ui_text_render);
  
  /* Scripting */
  
  scripting_init();
  
  printf("Done!\n");
  
}

void corange_finish() {
  
  scripting_finish();
  
  ui_manager_finish();
  entity_manager_finish();
  asset_manager_finish();
  
  viewport_finish();
  
  SDL_Quit();

}
