#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL.h"
#include "SDL_opengl.h"

#include "asset_manager.h"
#include "texture.h"

#include "bmp_loader.h"

texture* bmp_load_file(char* filename) {
  
  texture my_texture;	
  SDL_Surface *surface;
  GLenum texture_format;
  GLint  nOfColors;
   
  if ( (surface = SDL_LoadBMP(filename)) ) { 
   
    // Check that the image's width is a power of 2
    if ( (surface->w & (surface->w - 1)) != 0 ) {
      printf("warning: image.bmp's width is not a power of 2\n");
    }
   
    // Also check if the height is a power of 2
    if ( (surface->h & (surface->h - 1)) != 0 ) {
      printf("warning: image.bmp's height is not a power of 2\n");
    }
   
          // get the number of channels in the SDL surface
          nOfColors = surface->format->BytesPerPixel;
          if (nOfColors == 4)     // contains an alpha channel
          {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGBA;
            else
                    texture_format = GL_BGRA;
          } else if (nOfColors == 3)     // no alpha channel
          {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGB;
            else
                    texture_format = GL_BGR;
          } else {
            printf("warning: the image is not truecolor..  this will probably break\n");
            // this error should not go unhandled
          }
   
    // Have OpenGL generate a texture object handle for us
    glGenTextures(1, &my_texture);
   
    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, my_texture);
   
    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                        texture_format, GL_UNSIGNED_BYTE, surface->pixels);
  } 
  else {
    printf("SDL could not load image.bmp: %s\n", SDL_GetError());
    SDL_Quit();
    return NULL;
  }
   
  // Free the SDL_Surface only if it was successfully created
  if ( surface ) { 
    SDL_FreeSurface( surface );
  }
  
  texture* tex = malloc(sizeof(texture));
  *tex = my_texture;
  
  return tex;

}