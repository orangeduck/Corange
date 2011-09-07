
#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "asset_manager.h"
#include "texture.h"

void texture_delete(texture* t) {
  texture tex = *t;
  glDeleteTextures(1, &tex);
  free(t);
}

texture* texture_new() {
  
  texture* t = malloc(sizeof(texture));
  
  texture tex_id;
  glGenTextures(1, &tex_id);
  
  *t = tex_id;
  
  return t;
}


unsigned char pixels[2048 * 2048 * 4];
texture* last_texture = NULL;
int size_x, size_y;

/* Warning - due to the above allocation this can only sample 2048 by 2048 or under textures */

vector4 texture_sample(texture* t, vector2 point) {  
  
  if ( t != last_texture ) {
    
    glBindTexture(GL_TEXTURE_2D, *t);
   
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size_x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size_y);  
  
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    last_texture = t;
  }
  
  int u = size_x * point.x;
  int v = size_y * point.y;
  
  float r = (float)pixels[u * 4 + v * size_x * 4 + 0] / 255;
  float g = (float)pixels[u * 4 + v * size_x * 4 + 1] / 255;
  float b = (float)pixels[u * 4 + v * size_x * 4 + 2] / 255;
  float a = (float)pixels[u * 4 + v * size_x * 4 + 3] / 255;
  
  return v4(r,g,b,a);
}

void texture_write_to_file(texture* t, char* filename) {
  
  char* ext = asset_file_extension(filename);
  
  int format, width, height;
  
  glBindTexture(GL_TEXTURE_2D, *t);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format );
  
  if ( strcmp(ext,"bmp") == 0 ) {
    
  } else if ( strcmp(ext, "tga") == 0 ) {
    
    unsigned char* image_data = malloc( sizeof(unsigned char) * width * height * 4 );
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data ); 
    
    int xa= width % 256;
    int xb= (width-xa)/256;

    int ya= height % 256;
    int yb= (height-ya)/256;
    unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
    
    SDL_RWops* file = SDL_RWFromFile(filename, "wb");
    SDL_RWwrite(file, header, sizeof(header), 1);
    SDL_RWwrite(file, image_data, sizeof(char) * width * height * 4, 1 );
    SDL_RWclose(file);
    
    free(image_data);
    
  } else {
    
    printf("Error: Cannot save texture to &s, unknown file extension %s\n", filename, ext);
    
  }
  
  free(ext);
  
}

texture* bmp_load_file(char* filename) {
  
  printf("Loading: %s\n", filename);
  
  texture my_texture;	
  SDL_Surface *surface;
  GLenum texture_format;
  GLint  nOfColors;
   
  surface = SDL_LoadBMP(filename);
   
  if(!surface) {
    printf("Error: Could not load file %s: %s\n",filename , SDL_GetError());
    return NULL;
  }
  
  nOfColors = surface->format->BytesPerPixel;
  if (nOfColors == 4) {
    if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGBA;
    else texture_format = GL_BGRA;
  
  } else if (nOfColors == 3) {
    if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGB;
    else texture_format = GL_BGR;
  }
 
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );

  if (surface) { 
    SDL_FreeSurface(surface);
  }
  
  texture* tex = malloc(sizeof(texture));
  *tex = my_texture;
  
  return tex;
}

texture* load_image_file(char* filename) {
  
  printf("Loading: %s\n", filename);
  
  texture my_texture;	
  SDL_Surface *surface;
  GLenum texture_format;
  GLint  nOfColors;
   
  surface = IMG_Load(filename);
   
  if(!surface) {
    printf("Error: Could not load file %s: %s\n",filename , SDL_GetError());
    return NULL;
  }
  
  nOfColors = surface->format->BytesPerPixel;
  if (nOfColors == 4) {
    if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGBA;
    else texture_format = GL_BGRA;
  
  } else if (nOfColors == 3) {
    if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGB;
    else texture_format = GL_BGR;
  }
 
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );

  if (surface) { 
    SDL_FreeSurface(surface);
  }
  
  texture* tex = malloc(sizeof(texture));
  *tex = my_texture;
  
  return tex;
};

texture* png_load_file(char* filename) {
  return load_image_file(filename);
};

texture* tif_load_file(char* filename) {
  return load_image_file(filename);
};

texture* jpg_load_file(char* filename) {
  return load_image_file(filename);
};

DdsLoadInfo loadInfoDXT1 = {
  1, 0, 0, 4, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
};
DdsLoadInfo loadInfoDXT3 = {
  1, 0, 0, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
};
DdsLoadInfo loadInfoDXT5 = {
  1, 0, 0, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
};
DdsLoadInfo loadInfoBGRA8 = {
  0, 0, 0, 1, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE
};
DdsLoadInfo loadInfoBGR8 = {
  0, 0, 0, 1, 3, GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE
};
DdsLoadInfo loadInfoBGR5A1 = {
  0, 1, 0, 1, 2, GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV
};
DdsLoadInfo loadInfoBGR565 = {
  0, 1, 0, 1, 2, GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5
};
DdsLoadInfo loadInfoIndex8 = {
  0, 0, 1, 1, 1, GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE
};

#define max(A,B) (((A)>(B))?(A):(B))

texture* dds_load_file( char* filename ){
  
  printf("Loading: %s\n", filename);
  
  texture my_texture;
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);

  DDS_header hdr;
  size_t s = 0;
  int x = 0;
  int y = 0;
  int mipMapCount = 0;
  
  SDL_RWops* f = SDL_RWFromFile(filename, "rb");
  SDL_RWread(f, &hdr, 1, sizeof(hdr));
  
  if( hdr.dwMagic != DDS_MAGIC || hdr.dwSize != 124 ||
    !(hdr.dwFlags & DDSD_PIXELFORMAT) || !(hdr.dwFlags & DDSD_CAPS) ) {
    
    printf("Error Loading File %s: Does not appear to be a .dds file.\n", filename);
    return NULL;
  }

  x = hdr.dwWidth;
  y = hdr.dwHeight;

  DdsLoadInfo* li;

  if( PF_IS_DXT1( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT1;
  }
  else if( PF_IS_DXT3( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT3;
  }
  else if( PF_IS_DXT5( hdr.sPixelFormat ) ) {
    li = &loadInfoDXT5;
  }
  else if( PF_IS_BGRA8( hdr.sPixelFormat ) ) {
    li = &loadInfoBGRA8;
  }
  else if( PF_IS_BGR8( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR8;
  }
  else if( PF_IS_BGR5A1( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR5A1;
  }
  else if( PF_IS_BGR565( hdr.sPixelFormat ) ) {
    li = &loadInfoBGR565;
  }
  else if( PF_IS_INDEX8( hdr.sPixelFormat ) ) {
    li = &loadInfoIndex8;
  }
  else {
    printf("Error Loading File %s: Unknown DDS File format type.\n", filename);
    return NULL;
  }
  
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
  mipMapCount = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;
  
  int ix, zz;
  GLenum cFormat, format;
  
  if( li->compressed ) {
    size_t size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    char* data = malloc( size );
    
    if( !data ) {
      printf("Error Loading File %s: Does not appear to contain any data.\n", filename);
      return NULL;
    }
    
    cFormat = li->internalFormat;
    format = li->internalFormat;
    
    for( ix = 0; ix < mipMapCount; ++ix ) {
    
      SDL_RWread(f, data, 1, size);
      glCompressedTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, size, data );
      
      x = (x+1)>>1;
      y = (y+1)>>1;
      
      size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    }
    free( data );
    
  } else if( li->palette ) {
  
    size_t size = hdr.dwPitchOrLinearSize * y;
    format = li->externalFormat;
    cFormat = li->internalFormat;
    char* data = malloc( size );
    int palette[256];
    int* unpacked = malloc( size * sizeof(int) );
    
    SDL_RWread(f, palette, 4, 256);
    for( ix = 0; ix < mipMapCount; ++ix ) {
    
      SDL_RWread(f, data, 1, size);
      
      for( zz = 0; zz < size; ++zz ) {
        unpacked[ zz ] = palette[ data[ zz ] ];
      }
      
      glPixelStorei( GL_UNPACK_ROW_LENGTH, y );
      glTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, li->externalFormat, li->type, unpacked );
      
      x = (x+1)>>1;
      y = (y+1)>>1;
      
      size = x * y * li->blockBytes;
    }
    free( data );
    free( unpacked );
    
  } else {
  
    if( li->swap ) {
      glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_TRUE );
    }
    
    size_t size = x * y * li->blockBytes;
    format = li->externalFormat;
    cFormat = li->internalFormat;
    char * data = malloc( size );
    //fixme: how are MIP maps stored for 24-bit if pitch != ySize*3 ?
    for( ix = 0; ix < mipMapCount; ++ix ) {
    
      SDL_RWread(f, data, 1, size);
      
      glPixelStorei( GL_UNPACK_ROW_LENGTH, y );
      glTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, li->externalFormat, li->type, data );
      
      x = (x+1)>>1;
      y = (y+1)>>1;
      size = x * y * li->blockBytes;
    }
    free( data );
    glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_FALSE );
  }
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount-1 );
  
  texture* tex = malloc(sizeof(texture));
  *tex = my_texture;
  
  return tex;
  
}