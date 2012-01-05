#include "corange.h"

#include "spline.h"

int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  asset_manager_init();
  asset_manager_handler("acv", acv_load_file, color_curves_delete);
  
  load_folder("./input/");
  
  debug("Generating LUTS...");
  color_curves_write_lut(asset_get("./input/test.acv"), "./output/funky.lut");
  color_curves_write_lut(asset_get("./input/test2.acv"), "./output/identity.lut");
  color_curves_write_lut(asset_get("./input/bluey.acv"), "./output/bluey.lut");
  debug("Done!");
  
  asset_manager_finish();
  
  return 0;
}
