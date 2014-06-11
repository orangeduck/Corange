#include "assets/texture.h"

#include "data/spline.h"

texture* texture_new() {
  
  texture* t = malloc(sizeof(texture));
  glGenTextures(1, &t->handle);
  t->type = GL_TEXTURE_2D;
  
  return t;
}

void texture_delete(texture* t) {
  glDeleteTextures(1, &t->handle);
  free(t);
}

GLuint texture_handle(texture* t) {
  return t->handle;
}

GLenum texture_type(texture* t) {
  return t->type;
}

void texture_set_image(texture* t, image* i) {
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i->width, i->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, i->data );
}

texture* tga_load_file( char* filename ) {
  
  image* i = image_tga_load_file(filename);
  
  texture* t = texture_new();
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  
  texture_set_image(t, i);
  texture_set_filtering_anisotropic(t);
  
  image_delete(i);
  
  return t;
}

texture* bmp_load_file( char* filename ) {

  image* i = image_bmp_load_file(filename);
  
  texture* t = texture_new();
  glBindTexture(GL_TEXTURE_2D, texture_handle(t));
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
  
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
  float* data_flt;
  unsigned int* data_int;
  
  switch (format) {
    
    case GL_RGBA:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    break;
  
    case GL_ALPHA16:
    
      data_flt = malloc(sizeof(float) * width * height);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_FLOAT, data_flt);
        
      for(int x = 0; x < width; x++)
      for(int y = 0; y < height; y++) {
        data[(y*4*width) + (x*4) + 0] = pow(data_flt[(y*width) + x], 256.0) * 255;
        data[(y*4*width) + (x*4) + 1] = pow(data_flt[(y*width) + x], 256.0) * 255;
        data[(y*4*width) + (x*4) + 2] = pow(data_flt[(y*width) + x], 256.0) * 255;
        data[(y*4*width) + (x*4) + 3] = pow(data_flt[(y*width) + x], 256.0) * 255;
      }
        
      free(data_flt);
    
    break;
    
    case GL_RGBA32F:
    case GL_RGBA16F:
    
      data_flt = malloc(4 * sizeof(float) * width * height);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data_flt);
      
      for(int x = 0; x < width; x++)
      for(int y = 0; y < height; y++) {
        data[(y*4*width) + (x*4) + 0] = clamp(data_flt[(y*4*width) + (x*4) + 0] * 127 + 127, 0, 255);
        data[(y*4*width) + (x*4) + 1] = clamp(data_flt[(y*4*width) + (x*4) + 1] * 127 + 127, 0, 255);
        data[(y*4*width) + (x*4) + 2] = clamp(data_flt[(y*4*width) + (x*4) + 2] * 127 + 127, 0, 255);
        data[(y*4*width) + (x*4) + 3] = clamp(data_flt[(y*4*width) + (x*4) + 3] * 127 + 127, 0, 255);
      }
      
      free(data_flt);
    
    break;
    
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT24:
    
      data_int = malloc(sizeof(unsigned int) * width * height);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data_int);
      
      for(int x = 0; x < width; x++)
      for(int y = 0; y < height; y++) {
        data[(y*4*width) + (x*4) + 0] = data_int[(y*width) + x];
        data[(y*4*width) + (x*4) + 1] = data_int[(y*width) + x];
        data[(y*4*width) + (x*4) + 2] = data_int[(y*width) + x];
        data[(y*4*width) + (x*4) + 3] = data_int[(y*width) + x];
      }
      
      free(data_int);
    
    break;
    
    default:
      error("Can't convert that particular texture format %i to an image.", format);
      
  }
  
  SDL_GL_CheckError();
  
  image* i = image_new(width, height, data);
  
  free(data);
  
  return i;
}

void texture_generate_mipmaps(texture* t) {

  glBindTexture(t->type, texture_handle(t));
  glGenerateMipmap(t->type);
  
}

void texture_set_filtering_nearest(texture* t) {
  
  glBindTexture(t->type, texture_handle(t));
  glTexParameteri(t->type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(t->type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(t->type, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
  
}

void texture_set_filtering_linear(texture* t) {

  glBindTexture(t->type, texture_handle(t));
  glTexParameteri(t->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
  
}

void texture_set_filtering_anisotropic(texture* t) {

  float max = 0;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);

  glBindTexture(t->type, texture_handle(t));
  glTexParameteri(t->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAX_ANISOTROPY_EXT, max);

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
  
  texture* t = texture_new();
  t->type = GL_TEXTURE_3D;
  
  glBindTexture(t->type, texture_handle(t));
  glTexImage3D(t->type, 0, GL_RGB, lut_size, lut_size, lut_size, 0, GL_RGB, GL_UNSIGNED_BYTE, contents + offset);
  glTexParameteri(t->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  
  free(contents);
  
  return t;
  
}

void texture3d_write_to_file(texture* t, char* filename) {
  
  int t_width; 
  int t_height;
  int t_depth;
  
  glBindTexture(t->type, texture_handle(t));
  glGetTexLevelParameteriv(t->type, 0, GL_TEXTURE_WIDTH, &t_width);
  glGetTexLevelParameteriv(t->type, 0, GL_TEXTURE_HEIGHT, &t_height);
  glGetTexLevelParameteriv(t->type, 0, GL_TEXTURE_DEPTH, &t_depth);
  
  int width = t_width;
  int height = t_height * t_depth;
  
  unsigned char* data = malloc(width * height * 4);
  
  glGetTexImage(t->type, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  
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

  bool compressed;
  bool swap;
  bool palette;
  int div_size;
  int block_bytes;
  GLenum internal_format;
  GLenum external_format;
  GLenum type;
  
} DdsLoadInfo;


static bool is_power_of_two(unsigned int x) {
  while (((x % 2) == 0) && x > 1) { x /= 2; }
  return (x == 1);
}

texture* dds_load_file( char* filename ) {
  
  DdsLoadInfo loadInfoDXT1 =   { true,  false, false, 4, 8,  GL_COMPRESSED_RGBA_S3TC_DXT1 };
  DdsLoadInfo loadInfoDXT3 =   { true,  false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT3 };
  DdsLoadInfo loadInfoDXT5 =   { true,  false, false, 4, 16, GL_COMPRESSED_RGBA_S3TC_DXT5 };
  DdsLoadInfo loadInfoBGRA8 =  { false, false, false, 1, 4,  GL_RGBA8,   GL_BGRA, GL_UNSIGNED_BYTE };
  DdsLoadInfo loadInfoBGR8 =   { false, false, false, 1, 3,  GL_RGB8,    GL_BGR,  GL_UNSIGNED_BYTE };
  DdsLoadInfo loadInfoBGR5A1 = { false, true,  false, 1, 2,  GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV };
  DdsLoadInfo loadInfoBGR565 = { false, true,  false, 1, 2,  GL_RGB5,    GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 };
  DdsLoadInfo loadInfoIndex8 = { false, false, true,  1, 1,  GL_RGB8,    GL_BGRA, GL_UNSIGNED_BYTE };
  
  SDL_RWops* f = SDL_RWFromFile(filename, "rb");
  
  if (f == NULL) {
    error("Cannot load file %s", filename);
  }
  
  DDS_header hdr;
  SDL_RWread(f, &hdr, 1, sizeof(DDS_header));
  
  if( hdr.dwMagic != DDS_MAGIC || hdr.dwSize != 124 ||
    !(hdr.dwFlags & DDSD_PIXELFORMAT) || !(hdr.dwFlags & DDSD_CAPS) ) {
    error("Cannot Load File %s: Does not appear to be a .dds file.\n", filename);
  }

  int x = hdr.dwWidth;
  int y = hdr.dwHeight;
  int mip_map_num = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;
  
  if (!is_power_of_two(x)) { error("Texture %s with is %i pixels which is not a power of two!", filename, x); }
  if (!is_power_of_two(y)) { error("Texture %s height is %i pixels which is not a power of two!", filename, y); }
  
  DdsLoadInfo* li = &loadInfoDXT1;
  if      (PF_IS_DXT1(hdr.sPixelFormat  )) { li = &loadInfoDXT1;   }
  else if (PF_IS_DXT3(hdr.sPixelFormat  )) { li = &loadInfoDXT3;   }
  else if (PF_IS_DXT5(hdr.sPixelFormat  )) { li = &loadInfoDXT5;   } 
  else if (PF_IS_BGRA8(hdr.sPixelFormat )) { li = &loadInfoBGRA8;  }
  else if (PF_IS_BGR8(hdr.sPixelFormat  )) { li = &loadInfoBGR8;   }
  else if (PF_IS_BGR5A1(hdr.sPixelFormat)) { li = &loadInfoBGR5A1; }
  else if (PF_IS_BGR565(hdr.sPixelFormat)) { li = &loadInfoBGR565; } 
  else if (PF_IS_INDEX8(hdr.sPixelFormat)) { li = &loadInfoIndex8; }
  else { error("Cannot Load File %s: Unknown DDS File format type.", filename); }
  
  texture* t = texture_new();
  
  if (hdr.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) {
    t->type = GL_TEXTURE_CUBE_MAP;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_handle(t));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mip_map_num-1);
  } else {
    t->type = GL_TEXTURE_2D;
    glBindTexture(GL_TEXTURE_2D, texture_handle(t));
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mip_map_num-1);
    texture_set_filtering_anisotropic(t);
  }
   
  for (int i = 0; i < (t->type == GL_TEXTURE_CUBE_MAP ? 6 : 1); i++) {
  
    GLenum target = t->type;
    
    if (t->type == GL_TEXTURE_CUBE_MAP) {
      target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
    }
        
    int x = hdr.dwWidth;
    int y = hdr.dwHeight;
    int mip_map_num = (hdr.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.dwMipMapCount : 1;
    
    if ( li->compressed ) {
      
      size_t size = max(li->div_size, x) / li->div_size * max(li->div_size, y) / li->div_size * li->block_bytes;
      char* data = malloc(size);
      
      for(int ix = 0; ix < mip_map_num; ix++) {
      
        SDL_RWread(f, data, 1, size);
        glCompressedTexImage2D(target, ix, li->internal_format, x, y, 0, size, data);
        
        x = (x+1)>>1;
        y = (y+1)>>1;
        
        size = max(li->div_size, x) / li->div_size * max(li->div_size, y) / li->div_size * li->block_bytes;
      }
      
      free(data);
      
    } else if ( li->palette ) {
      
      size_t size = hdr.dwPitchOrLinearSize * y;
      char* data = malloc(size);
      int palette[256];
      int* unpacked = malloc(size * sizeof(int));
      
      SDL_RWread(f, palette, 4, 256);
      for(int ix = 0; ix < mip_map_num; ix++) {
      
        SDL_RWread(f, data, 1, size);
        
        for( int zz = 0; zz < size; ++zz ) {
          unpacked[ zz ] = palette[ (short)data[ zz ] ];
        }
        
        glPixelStorei(GL_UNPACK_ROW_LENGTH, y);
        glTexImage2D(target, ix, li->internal_format, x, y, 0, li->external_format, li->type, unpacked);
        
        x = (x+1)>>1;
        y = (y+1)>>1;
        
        size = x * y * li->block_bytes;
      }
      
      free(data);
      free(unpacked);
      
    } else {
    
      if (li->swap) { glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE); }
      
      size_t size = x * y * li->block_bytes;
      char* data = malloc(size);
      
      for (int ix = 0; ix < mip_map_num; ix++) {
      
        SDL_RWread(f, data, 1, size);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, y);
        glTexImage2D(target, ix, li->internal_format, x, y, 0, li->external_format, li->type, data);
        
        x = (x+1)>>1;
        y = (y+1)>>1;
        size = x * y * li->block_bytes;
      }
      
      free(data);
      
      if (li->swap) { glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE); }
      
    }
    
  }
  
  SDL_RWclose(f);
  
  SDL_GL_CheckError();
  
  return t;
  
}

texture* acv_load_file( char* filename ) {

  color_curves* cc = color_curves_load(filename);
  
  uint32_t lut_size = 64;
  
  unsigned char* lut_data = malloc(sizeof(char) * 3 * lut_size * lut_size * lut_size);
  
  int r, g, b;
  for(r = 0; r < lut_size; r++)
  for(g = 0; g < lut_size; g++)
  for(b = 0; b < lut_size; b++) {
    
    int i = (3 * r) + (3 * lut_size * g) + (3 * lut_size * lut_size * b);
    
    float red   = (float)r / lut_size;
    float green = (float)g / lut_size;
    float blue  = (float)b / lut_size;
    
    red = spline_get_x(cc->r_spline, red);
    green = spline_get_x(cc->g_spline, green);
    blue = spline_get_x(cc->b_spline, blue);
    
    red = spline_get_x(cc->rgb_spline, red);
    green = spline_get_x(cc->rgb_spline, green);
    blue = spline_get_x(cc->rgb_spline, blue);
    
    lut_data[i+0] = (unsigned char) (red   * 255);
    lut_data[i+1] = (unsigned char) (green * 255);
    lut_data[i+2] = (unsigned char) (blue  * 255);
  
  }
  
  color_curves_delete(cc);

  texture* t = texture_new();
  t->type = GL_TEXTURE_3D;
  
  glBindTexture(t->type, texture_handle(t));
  glTexImage3D(t->type, 0, GL_RGB, lut_size, lut_size, lut_size, 0, GL_RGB, GL_UNSIGNED_BYTE, lut_data);
  glTexParameteri(t->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(t->type, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  
  free(lut_data);
  
  return t;
  
}
