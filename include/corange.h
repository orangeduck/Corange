/* Include SDL, OpenGL */

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

/* Include OpenCL */

#include "CL/cl.h"

/* Info and Utilities */

#include "timing.h"
#include "error.h"
#include "scripting.h"
#include "viewport.h"

/* Engine Entities */

#include "entity_manager.h"

#include "static_object.h"
#include "camera.h"
#include "light.h"

/* Engine Objects */

#include "renderable.h"
#include "image.h"

/* Engine Assets */

#include "asset_manager.h"

#include "font.h"
#include "material.h"
#include "shader.h"
#include "texture.h"

/* Rendering */

#include "forward_renderer.h"
#include "deferred_renderer.h"
#include "shadow_mapper.h"

/* UI */

#include "ui_text.h"
#include "ui_rectangle.h"

/* Data Structures */

#include "list.h"
#include "int_list.h"
#include "vertex_list.h"
#include "vertex_hashtable.h"
#include "dictionary.h"

/* Maths */

#include "vector.h"
#include "matrix.h"
#include "geometry.h"

/* Importers */

#include "obj_loader.h"

/* Processing */

#include "kernel.h"
