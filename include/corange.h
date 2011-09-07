/* Include SDL, OpenGL and GLEW */

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

/* Main Engine Files */

#include "asset_manager.h"

/* Info and Utilities */

#include "timing.h"
#include "logger.h"
#include "scripting.h"
#include "viewport.h"

/* Engine Entities */

#include "camera.h"
#include "light.h"
#include "font.h"
#include "material.h"
#include "shader.h"
#include "texture.h"

/* Rendering */

#include "renderable.h"
#include "forward_renderer.h"
#include "deferred_renderer.h"
#include "text_renderer.h"
#include "shadow_mapper.h"

/* Data Structures */

#include "list.h"
#include "int_list.h"
#include "vertex_list.h"
#include "dictionary.h"
#include "vertex_hashtable.h"

/* Maths */

#include "vector.h"
#include "matrix.h"
#include "geometry.h"

/* Importers */

#include "obj_loader.h"

/* Misc */

#include "perlin_noise.h"