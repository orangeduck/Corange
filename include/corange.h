/**
*** :: Corange ::
***
***   Pure and Simple game engine written in C 
***   
***   Uses SDL as a bottom layer and OpenGL for rendering
***   Provides asset, UI and entity management.
***   As well as deferred and forward renderers.
***   And a host of small demos.
***
***   Suggestions and contributions welcome:
***
***     Daniel Holden | contact@theorangeduck.com
***     
***     https://github.com/orangeduck/Corange
***
**/

#ifndef corange_h
#define corange_h

/* Core engine modules */

#include "cengine.h"
#include "cgraphics.h"
#include "caudio.h"

/* Corange Functions */

void corange_init(const char* core_assets_path);
void corange_finish();

/* Entities */

#include "centity.h"

#include "entities/camera.h"
#include "entities/light.h"
#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/physics_object.h"
#include "entities/instance_object.h"
#include "entities/landscape.h"

/* Assets */

#include "casset.h"

#include "assets/image.h"
#include "assets/sound.h"
#include "assets/font.h"
#include "assets/shader.h"
#include "assets/texture.h"
#include "assets/material.h"
#include "assets/renderable.h"
#include "assets/collision_body.h"
#include "assets/skeleton.h"
#include "assets/animation.h"
#include "assets/terrain.h"

/* UI */

#include "cui.h"

#include "ui/ui_text.h"
#include "ui/ui_rectangle.h"
#include "ui/ui_spinner.h"
#include "ui/ui_button.h"

/* Rendering */

#include "rendering/forward_renderer.h"
#include "rendering/deferred_renderer.h"
#include "rendering/shadow_mapper.h"
#include "rendering/depth_mapper.h"

/* Data Structures */

#include "data/dict.h"
#include "data/list.h"
#include "data/int_list.h"
#include "data/vertex_list.h"
#include "data/vertex_hashtable.h"

#endif
