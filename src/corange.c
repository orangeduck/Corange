#include <stdlib.h>

#include "corange.h"

void corange_init(char* core_assets_path) {
  
  /* Stop stdout redirect on windows */
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  /* Starting Corange */
  
  debug("Starting Corange...");
  
  /* Init OpenGL and Viewport */
  
  debug("Starting Graphics...");
  viewport_init();
  
  debug("Starting Audio...");
  audio_mixer_init();
  
  /* Asset Manager */
  
  debug("Creating Asset Manager...");
  
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
  asset_manager_handler("col", col_load_file, collision_body_delete);
  
  asset_manager_handler("bmp", bmp_load_file, image_delete);
  asset_manager_handler("tga", tga_load_file, image_delete);
  asset_manager_handler("dds", dds_load_file, texture_delete);
  asset_manager_handler("lut", lut_load_file, texture_delete);
  
  asset_manager_handler("vs" , vs_load_file,  shader_delete);
  asset_manager_handler("fs" , fs_load_file,  shader_delete);
  asset_manager_handler("prog",prog_load_file,shader_program_delete);
  
  asset_manager_handler("fnt", font_load_file,font_delete);
  asset_manager_handler("mat", mat_load_file, material_delete);
  asset_manager_handler("mmat", mmat_load_file, multi_material_delete);
  
  asset_manager_handler("wav", wav_load_file, sound_delete);
  
  /* Entity Manager */
  
  debug("Creating Entity Manager...");
  
  entity_manager_init();
  
  entity_manager_handler(static_object, static_object_new, static_object_delete);
  entity_manager_handler(animated_object, animated_object_new, animated_object_delete);
  entity_manager_handler(physics_object, physics_object_new, physics_object_delete);
  entity_manager_handler(camera, camera_new, camera_delete);
  entity_manager_handler(light, light_new, light_delete);
  
  /* UI Manager */
  
  debug("Creating UI Manager...");
  
  ui_manager_init();
  
  ui_manager_handler(ui_rectangle, ui_rectangle_new, ui_rectangle_delete, ui_rectangle_update, ui_rectangle_render);
  ui_manager_handler(ui_text, ui_text_new, ui_text_delete, ui_text_update, ui_text_render);
  ui_manager_handler(ui_spinner, ui_spinner_new, ui_spinner_delete, ui_spinner_update, ui_spinner_render);
  ui_manager_handler(ui_button, ui_button_new, ui_button_delete, ui_button_update, ui_button_render);
  
  debug("Finished");
  
}

void corange_finish() {
  
  ui_manager_finish();
  entity_manager_finish();
  asset_manager_finish();
  
  audio_mixer_finish();
  
  viewport_finish();
  
  SDL_Quit();

}
