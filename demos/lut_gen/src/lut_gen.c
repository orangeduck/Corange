#include "corange.h"

#include "spline.h"

int main(int argc, char **argv) {
  
  corange_stop_stdout_redirect();
  
  asset_manager_init();
  asset_manager_handler("acv", acv_load_file, color_curves_delete);
  
  load_folder("./input/");
  
  printf("Generating LUTS...\n");
  color_curves_write_lut(asset_get("./input/test.acv"), "./output/funky.lut");
  color_curves_write_lut(asset_get("./input/test2.acv"), "./output/identity.lut");
  color_curves_write_lut(asset_get("./input/bluey.acv"), "./output/bluey.lut");
  printf("Done!\n\n");
  
  asset_manager_finish();
}