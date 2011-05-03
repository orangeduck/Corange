
#include "texture.h"

void texture_delete(texture* t) {
  texture tex = *t;
  glDeleteTextures(1, &tex);
  free(t);
}