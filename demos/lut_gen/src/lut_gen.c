#include "corange.h"

#include "spline.h"

int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  if (argc != 3) {
    printf("Useage: lut_gen <input.acv> <output.lut>\n");
  } else {
    debug("Loading File: %s", argv[1]);
    color_curves* curves = acv_load_file(argv[1]);
    debug("Writing File: %s", argv[2]);
    color_curves_write_lut(curves, argv[2]);
    debug("Done!");
  }
  
  return 0;
  
}