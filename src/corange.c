#include <stdlib.h>
#include <signal.h>

#include "corange.h"

static void corange_signal(int sig) {
  switch(sig) {
    case SIGABRT: error("Program Aborted");
    case SIGFPE: error("Division by Zero");
    case SIGILL: error("Illegal Instruction");
    case SIGINT: error("Program Interrupted");
    case SIGSEGV: error("Segmentation fault");
    case SIGTERM: error("Program Terminated");
  }
}

void corange_init(char* core_assets_path) {
  
  /* Stop stdout redirect on windows */
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  /* Attach signal handlers */
  signal(SIGABRT, corange_signal);
  signal(SIGFPE, corange_signal);
  signal(SIGILL, corange_signal);
  signal(SIGINT, corange_signal);
  signal(SIGSEGV, corange_signal);
  signal(SIGTERM, corange_signal);
  
  aterror(SDL_PrintStackTrace);
  
  /* Starting Corange */
  debug("Starting Corange...");
  
  /* Graphics Manager */
  debug("Creating Graphics Manager...");
  graphics_manager_init();
  
  /* Audio Manager */
  debug("Creating Audio Manager...");
  audio_manager_init();
  
  /* Asset Manager */
  debug("Creating Asset Manager...");
  
  asset_manager_init();
  asset_manager_add_path_variable("$CORANGE", core_assets_path);
  
  asset_manager_handler(renderable, "obj", obj_load_file, renderable_delete);
  asset_manager_handler(renderable, "smd", smd_load_file, renderable_delete);
  asset_manager_handler(skeleton, "skl", skl_load_file, skeleton_delete);
  asset_manager_handler(animation, "ani", ani_load_file, animation_delete);
  asset_manager_handler(collision_body, "col", col_load_file, collision_body_delete);
  asset_manager_handler(terrain, "raw", raw_load_file, terrain_delete);
  
  asset_manager_handler(image, "bmp", bmp_load_file, image_delete);
  asset_manager_handler(image, "tga", tga_load_file, image_delete);
  asset_manager_handler(texture, "dds", dds_load_file, texture_delete);
  asset_manager_handler(texture, "lut", lut_load_file, texture_delete);
  
  asset_manager_handler(shader, "vs" , vs_load_file,  shader_delete);
  asset_manager_handler(shader, "fs" , fs_load_file,  shader_delete);
  asset_manager_handler(shader, "gs" , gs_load_file,  shader_delete);
  asset_manager_handler(shader_program, "prog", prog_load_file, shader_program_delete);
  
  asset_manager_handler(font, "fnt", font_load_file, font_delete);
  asset_manager_handler(material, "mat", mat_load_file, material_delete);
  asset_manager_handler(multi_material, "mmat", mmat_load_file, multi_material_delete);
  
  asset_manager_handler(sound, "wav", wav_load_file, sound_delete);
  
  /* Entity Manager */
  debug("Creating Entity Manager...");
  
  entity_manager_init();
  
  entity_manager_handler(static_object, static_object_new, static_object_delete);
  entity_manager_handler(animated_object, animated_object_new, animated_object_delete);
  entity_manager_handler(physics_object, physics_object_new, physics_object_delete);
  entity_manager_handler(instance_object, instance_object_new, instance_object_delete);
  
  entity_manager_handler(camera, camera_new, camera_delete);
  entity_manager_handler(light, light_new, light_delete);
  entity_manager_handler(landscape, landscape_new, landscape_delete);
  
  /* UI Manager */
  debug("Creating UI Manager...");
  
  ui_manager_init();
  
  ui_manager_handler(ui_rectangle, ui_rectangle_new, ui_rectangle_delete, ui_rectangle_update, ui_rectangle_render);
  ui_manager_handler(ui_text, ui_text_new, ui_text_delete, ui_text_update, ui_text_render);
  ui_manager_handler(ui_spinner, ui_spinner_new, ui_spinner_delete, ui_spinner_update, ui_spinner_render);
  ui_manager_handler(ui_button, ui_button_new, ui_button_delete, ui_button_update, ui_button_render);
  
  debug("Finished!");
}

void corange_finish() {
  
  ui_manager_finish();
  entity_manager_finish();
  asset_manager_finish();
  
  audio_manager_finish();
  graphics_manager_finish();
  
  SDL_Quit();

}
