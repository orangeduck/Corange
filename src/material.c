#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asset_manager.h"

#include "material.h"

material* material_new(char* name) {

  material* mat = malloc(sizeof(material));
  
  mat->name = malloc(strlen(name) + 1); strcpy(mat->name, name);
  mat->properties = dictionary_new(20);

  return mat;
  
};

material* mat_load_file(char* filename) {

  material* mat = material_new("boobies");

  char* c = asset_load_file(filename);
  
  printf("Loading a material!\n");
  
  char* line = malloc(1024);
  
  int i = 0;
  int j = 0;
  
  while(1) {
  
    /* If end of string then exit. */
    if( c[i] == '\0') { break; }
    
    /* End of line reached */
    if( c[i] == '\n') {
    
      /* Null terminate line buffer */
      line[j-1] = '\0';
      
      //printf("LINE: %s \n",line);

      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      /* Otherwise add character to line buffer */
      line[j] = c[i];
      j++;
    }
    i++;
  }
  
  free(line);
  
  free(c);
  
  return mat;

};

void material_delete(material* mat) {

  free(mat->name);
  dictionary_delete(mat->properties);
  free(mat);
  
};