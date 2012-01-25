#ifndef corange_h
#define corange_h

/* Corange Functions */

void corange_init(char* core_assets_path);
void corange_finish();

/* Include SDL, OpenGL */

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

/* Info and Utilities */

#include "bool.h"
#include "error.h"
#include "timing.h"

/* Entities */

#include "entity_manager.h"

#include "static_object.h"
#include "animated_object.h"
#include "camera.h"
#include "light.h"

/* Assets */

#include "asset_manager.h"

#include "animation.h"
#include "skeleton.h"
#include "renderable.h"
#include "collision_body.h"
#include "font.h"
#include "material.h"
#include "shader.h"
#include "texture.h"
#include "image.h"
#include "sound.h"

/* UI */

#include "ui_manager.h"

#include "ui_text.h"
#include "ui_rectangle.h"
#include "ui_spinner.h"

/* Rendering */

#include "viewport.h"
#include "forward_renderer.h"
#include "deferred_renderer.h"
#include "shadow_mapper.h"

/* Audio */

#include "audio_mixer.h"

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

#endif
