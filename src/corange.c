#include <stdlib.h>

#include "corange.h"

static int path_set = 0;

#define MAX_PATH_LEN 2048
static char corange_path[MAX_PATH_LEN];

char* corange_asset_path() {
  if(!path_set) {
    return NULL;
  } else {
    return corange_path;
  }
}

void corange_stop_stdout_redirect() {
  FILE* ctt = fopen("CON", "w" );
  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
}

void corange_init(char* core_assets_path) {
  
  printf("Starting Corange...\n");
  
  /* Stop Redirect of stdout and stderr */
  
  corange_stop_stdout_redirect();
  
  /* Load asset path stuff */
  
  if (strlen(core_assets_path) >= MAX_PATH_LEN) {
    error("core assets path too long");
  } else {
    strcpy(corange_path, core_assets_path);
    path_set = 1;
  }
  
  /* TODO: Check path is valid */
  
  /* SDL Setup */
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    error("Unable to initialize SDL: %s\n", SDL_GetError());
  }
  
  viewport_init();
  
  SDL_LoadOpenGLExtensions();

  /* Load Assets */
  
  asset_manager_init();
  
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
