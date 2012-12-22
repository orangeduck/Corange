#include "assets/texture.h"

texture* texture_new() {
  
  texture* t = malloc(sizeof(texture));
  
  texture tex_id;
  glGenTextures(1, &tex_id);
  
  *t = tex_id;
  
  return t;
}

void texture_delete(texture* t) {
  texture tex = *t;
  glDeleteTextures(1, &tex);
  free(t);
}

GLuint texture_handle(texture* t) {
  if (t == NULL) {
    error("Cannot get handle for NULL texture");
  }
  
  return *t;
}

void texture_set_image(texture* t, image* i) {
  
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i->width, i->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, i->data );
  
}

texture* tga_load_file( char* filename ) {
  
  image* i = image_tga_load_file(filename);
  
  texture my_texture;
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
  
  texture* t = malloc(sizeof(texture));
  *t = my_texture;
  
  texture_set_image(t, i);
  texture_set_filtering_anisotropic(t);
  
  image_delete(i);
  
  return t;
}

texture* bmp_load_file( char* filename ) {

  image* i = image_bmp_load_file(filename);
  
  texture my_texture;
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
  
  texture* t = malloc(sizeof(texture));
  *t = my_texture;
  
  texture_set_image(t, i);
  texture_set_filtering_anisotropic(t);
  
  image_delete(i);
  
  return t;
}

image* texture_get_image(texture* t) {
  
  int width = 0;
  int height = 0;
  int format = 0;
  
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
  
  if ((width == 0) || (height == 0)) {
    error("Texture has zero size width/height: (%i, %i)", width, height);
  }
  
  unsigned char* data = malloc(width * height * 4);
  
  if (format == GL_RGBA) {
  
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
  } else if (format == GL_ALPHA16) {
    
    float* depth_data = malloc(sizeof(float) * width * height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_FLOAT, depth_data);
      
    for(int x = 0; x < width; x++)
    for(int y = 0; y < height; y++) {
      
      float depth = depth_data[(y*width) + x];
      depth = pow(depth, 256.0);
      
      data[(y*4*width) + (x*4) + 0] = depth * 255;
      data[(y*4*width) + (x*4) + 1] = depth * 255;
      data[(y*4*width) + (x*4) + 2] = depth * 255;
      data[(y*4*width) + (x*4) + 3] = depth * 255;
    }
      
    free(depth_data);
    
  } else if (format == GL_RGBA32F) {
    
    float* pos_data = malloc(4 * sizeof(float) * width * height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pos_data);
    
    for(int x = 0; x < width; x++)
    for(int y = 0; y < height; y++) {
      data[(y*4*width) + (x*4) + 0] = clamp(pos_data[(y*4*width) + (x*4) + 0] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 1] = clamp(pos_data[(y*4*width) + (x*4) + 1] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 2] = clamp(pos_data[(y*4*width) + (x*4) + 2] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 3] = clamp(pos_data[(y*4*width) + (x*4) + 3] * 255, 0, 255);
    }
    
    free(pos_data);
  
  } else if (format == GL_RGBA16F) {
    
    float* norm_data = malloc(4 * sizeof(float) * width * height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, norm_data);
    
    for(int x = 0; x < width; x++)
    for(int y = 0; y < height; y++) {
      data[(y*4*width) + (x*4) + 0] = clamp(norm_data[(y*4*width) + (x*4) + 0] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 1] = clamp(norm_data[(y*4*width) + (x*4) + 1] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 2] = clamp(norm_data[(y*4*width) + (x*4) + 2] * 255, 0, 255);
      data[(y*4*width) + (x*4) + 3] = clamp(norm_data[(y*4*width) + (x*4) + 3] * 255, 0, 255);
    }
    
    free(norm_data);
    
  } else if (format == GL_DEPTH_COMPONENT) {
    
    unsigned int* depth_data = malloc(sizeof(unsigned int) * width * height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, depth_data);
    
    for(int x = 0; x < width; x++)
    for(int y = 0; y < height; y++) {
      
      unsigned int depth = depth_data[(y*width) + x];
      
      data[(y*4*width) + (x*4) + 0] = depth;
      data[(y*4*width) + (x*4) + 1] = depth;
      data[(y*4*width) + (x*4) + 2] = depth;
      data[(y*4*width) + (x*4) + 3] = depth;
    }
    
    free(depth_data);
  } else {
    error("Can't save that particular texture format %i to file.", format);
  }
  
  image* i = image_new(width, height, data);
  
  free(data);
  
  return i;
}

void texture_generate_mipmaps(texture* t) {

  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glGenerateMipmap(GL_TEXTURE_2D);
  
}

void texture_set_filtering_nearest(texture* t) {
  
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
  
}

void texture_set_filtering_linear(texture* t) {

  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
  
}

void texture_set_filtering_anisotropic(texture* t) {

  float max;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);

  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max);

}


void texture_write_to_file(texture* t, char* filename){
  
  image* i = texture_get_image(t);
  image_write_to_file(i, filename);
  image_delete(i);
  
}

texture* lut_load_file( char* filename ) {
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  long size = SDL_RWseek(file,0,SEEK_END);
  unsigned char* contents = malloc(size+1);
  contents[size] = '\0';
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, size, 1);
  
  SDL_RWclose(file);
  
  int head = sizeof("CORANGE-LUT")-1;
  int lut_size = (unsigned char)contents[head] | (unsigned char)contents[head + 1];
  
  int offset = head + 3;
  
  texture* t = malloc(sizeof(texture));
  
  texture tex_id;
  glEnable(GL_TEXTURE_3D);
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_3D, tex_id);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, lut_size, lut_size, lut_size, 0, GL_RGB, GL_UNSIGNED_BYTE, contents + offset);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  glDisable(GL_TEXTURE_3D);
  
  *t = tex_id;
  
  free(contents);
  
  return t;
  
}

void texture3d_write_to_file(texture* t, char* filename) {
  
  int t_width; 
  int t_height;
  int t_depth;
  
  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, *t);
  glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &t_width);
  glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &t_height);
  glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &t_depth);
  
  int width = t_width;
  int height = t_height * t_depth;
  
  unsigned char* data = malloc(width * height * 4);
  
  glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glDisable(GL_TEXTURE_3D);
  
  int xa= width % 256;
  int xb= (width-xa)/256;

  int ya= height % 256;
  int yb= (height-ya)/256;
  unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
  
  SDL_RWops* file = SDL_RWFromFile(filename, "wb");
  SDL_RWwrite(file, header, sizeof(header), 1);
  SDL_RWwrite(file, data, width * height * 4, 1 );
  SDL_RWclose(file);
  
  free(data);
}

/* DDS file stuff */

/* little-endian, of course */
#define DDS_MAGIC 0x20534444

/* DDS_header.dwFlags */
#define DDSD_CAPS                   0x00000001 
#define DDSD_HEIGHT                 0x00000002 
#define DDSD_WIDTH                  0x00000004 
#define DDSD_PITCH                  0x00000008 
#define DDSD_PIXELFORMAT            0x00001000 
#define DDSD_MIPMAPCOUNT            0x00020000 
#define DDSD_LINEARSIZE             0x00080000 
#define DDSD_DEPTH                  0x00800000 

/* DDS_header.sPixelFormat.dwFlags */
#define DDPF_ALPHAPIXELS            0x00000001 
#define DDPF_FOURCC                 0x00000004 
#define DDPF_INDEXED                0x00000020 
#define DDPF_RGB                    0x00000040 

/* DDS_header.sCaps.dwCaps1 */
#define DDSCAPS_COMPLEX             0x00000008 
#define DDSCAPS_TEXTURE             0x00001000 
#define DDSCAPS_MIPMAP              0x00400000 

/* DDS_header.sCaps.dwCaps2 */
#define DDSCAPS2_CUBEMAP            0x00000200 
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400 
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800 
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000 
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000 
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000 
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000 
#define DDSCAPS2_VOLUME             0x00200000 

#define D3DFMT_DXT1     0x31545844    /* DXT1 compression texture format */
#define D3DFMT_DXT2     0x32545844    /* DXT2 compression texture format */
#define D3DFMT_DXT3     0x33545844    /* DXT3 compression texture format */
#define D3DFMT_DXT4     0x34545844    /* DXT4 compression texture format */
#define D3DFMT_DXT5     0x35545844    /* DXT5 compression texture format */

#define PF_IS_DXT1(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT1))

#define PF_IS_DXT3(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT3))

#define PF_IS_DXT5(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT5))

#define PF_IS_BGRA8(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
   (pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 32) && \
   (pf.dwRBitMask == 0xff0000) && \
   (pf.dwGBitMask == 0xff00) && \
   (pf.dwBBitMask == 0xff) && \
   (pf.dwAlphaBitMask == 0xff000000U))

#define PF_IS_BGR8(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
  !(pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 24) && \
   (pf.dwRBitMask == 0xff0000) && \
   (pf.dwGBitMask == 0xff00) && \
   (pf.dwBBitMask == 0xff))

#define PF_IS_BGR5A1(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
   (pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 16) && \
   (pf.dwRBitMask == 0x00007c00) && \
   (pf.dwGBitMask == 0x000003e0) && \
   (pf.dwBBitMask == 0x0000001f) && \
   (pf.dwAlphaBitMask == 0x00008000))

#define PF_IS_BGR565(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
  !(pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 16) && \
   (pf.dwRBitMask == 0x0000f800) && \
   (pf.dwGBitMask == 0x000007e0) && \
   (pf.dwBBitMask == 0x0000001f))

#define PF_IS_INDEX8(pf) \
  ((pf.dwFlags & DDPF_INDEXED) && \
   (pf.dwRGBBitCount == 8))


typedef struct {
  unsigned int    dwMagic;
  unsigned int    dwSize;
  unsigned int    dwFlags;
  unsigned int    dwHeight;
  unsigned int    dwWidth;
  unsigned int    dwPitchOrLinearSize;
  unsigned int    dwDepth;
  unsigned int    dwMipMapCount;
  unsigned int    dwReserved1[ 11 ];

  /* DDPIXELFORMAT */
  struct {
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwFourCC;
    unsigned int    dwRGBBitCount;
    unsigned int    dwRBitMask;
    unsigned int    dwGBitMask;
    unsigned int    dwBBitMask;
    unsigned int    dwAlphaBitMask;
  } sPixelFormat;

  /* DDCAPS2 */
  struct {
    unsigned int    dwCaps1;
    unsigned int    dwCaps2;
    unsigned int    dwDDSX;
    unsigned int    dwReserved;
  } sCaps;
  
  unsigned int dwReserved2;
} DDS_header ;

typedef struct {

  int compressed;
  int swap;
  int palette;
  int divSize;
  int blockBytes;
  GLenum internalFormat;
  GLenum externalFormat;
  GLenum type;
  
} DdsLoadInfo;


static bool is_power_of_two(unsigned int x) {
 while (((x % 2) == 0) && x > 1) /* While x is even and > 1 */
   x /= 2;
 return (x == 1);
}

texture* dds_load_file( char* filename ) {
  
  SDL_GL_CheckError();
  
  DdsLoadInfo loadInfoDXT1 =   { 1, 0, 0, 4, 8,  GL_COMPRESSED_RGBA_S3TC_DXT1 };
  DdsLoadInfo loadInfoDXT3 =   { 1, 0, 0, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3 };
  DdsLoadInfo loadInfoDXT5 =   { 1, 0, 0, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5 };
  DdsLoadInfo loadInfoBGRA8 =  { 0, 0, 0, 1, 4,  GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE };
  DdsLoadInfo loadInfoBGR8 =   { 0, 0, 0, 1, 3,  GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE };
  DdsLoadInfo loadInfoBGR5A1 = { 0, 1, 0, 1, 2,  GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV };
  DdsLoadInfo loadInfoBGR565 = { 0, 1, 0, 1, 2,  GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 };
  DdsLoadInfo loadInfoIndex8 = { 0, 0, 1, 1, 1,  GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE };
  
  SDL_RWops* f = SDL_RWFromFile(filename, "rb");
  
  if (f == NULL) {
    error("Cannot load file %s", filename);
  }
  
  DDS_header hdr;
  SDL_RWread(f, &hdr, 1, sizeof(hdr));
  
  if( hdr.dwMagic != DDS_MAGIC || hdr.dwSize != 124 ||
    !(hdr.dwFlags & DDSD_PIXELFORMAT) || !(hdr.dwFlags & DDSD_CAPS) ) {
    error("Cannot Load File %s: Does not appear to be a .dds file.\n", filename);
  }

  int x = hdr.dwWidth;
  int y = hdr.dwHeight;
  
  if (!is_power_of_two(x)) { error("Texture %s with is %i pixels which is not a power of two!", filename, x); }
  if (!is_power_of_two(y)) { error("Texture %s height is %i pixels which is not a power of two!", filename, y); }
  
  DdsLoadInfo* li = &loadInfoDXT1;

  if( PF_IS_DXT1( hdr.sPixelFormat ) ) { li = &loadInfoDXT1; }
  else if( PF_IS_DXT3( hdr.sPixelFormat ) ) { li = &loadInfoDXT3; }
  else if( PF_IS_DXT5( hdr.sPixelFormat ) ) { li = &loadInfoDXT5; } 
  else if( PF_IS_BGRA8( hdr.sPixelFormat ) ) { li = &loadInfoBGRA8; }
  else if( PF_IS_BGR8( hdr.sPixelFormat ) ) { li = &loadInfoBGR8; }
  else if( PF_IS_BGR5A1( hdr.sPixelFormat ) ) { li = &loadInfoBGR5A1; }
  else if( PF_IS_BGR565( hdr.sPixelFormat ) ) { li = &loadInfoBGR565; } 
  else if( PF_IS_INDEX8( hdr.sPixelFormat ) ) { li = &loadInfoIndex8; }
  else { error("Cannot Load File %s: Unknown DDS File format type.", filename); }
  
  texture my_texture;
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
  
  int mipMapCount = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;
  
  if( li->compressed ) {
    
    size_t size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    char* data = malloc( size );
    
    GLenum cFormat = li->internalFormat;
    GLenum format = li->internalFormat;
    
    for( int ix = 0; ix < mipMapCount; ++ix ) {
    
      SDL_RWread(f, data, 1, size);
      glCompressedTexImage2D( GL_TEXTURE_2D, ix, li->internalFormat, x, y, 0, size, data );
      
      x = (x+1)>>1;
      y = (y+1)>>1;
      
      size = max( li->divSize, x )/li->divSize * max( li->divSize, y )/li->divSize * li->blockBytes;
    }
    
    free( data );
    
  } else if( li->palette ) {
  
    size_t size = hdr.dwPitchOrLinearSize * y;
    GLenum format = li->externalFormat;
    GLenum cFormat = li->internalFormat;
    char* data = malloc( size );
    int palette[256];
    int* unpacked = malloc( size * sizeof(int) );
    
    SDL_RWread(f, palette, 4, 256);
    for( int ix = 0; ix < mipMapCount; ++ix ) {
    
      SDL_RWread(f, data, 1, size);
      
      for( int zz = 0; zz < size; ++zz ) {
        unpacked[ zz ] = palette[ (short)data[ zz ] ];
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
  
    if( li->swap ) { glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_TRUE ); }
    
    size_t size = x * y * li->blockBytes;
    GLenum format = li->externalFormat;
    GLenum cFormat = li->internalFormat;
    char* data = malloc( size );
    
    /* fixme: how are MIP maps stored for 24-bit if pitch != ySize*3 ? */
    for( int ix = 0; ix < mipMapCount; ++ix ) {
    
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
  
  texture_set_filtering_anisotropic(tex);
  
  return tex;
  
}
