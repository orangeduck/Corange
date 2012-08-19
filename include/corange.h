#ifndef corange_h
#define corange_h

#include "cengine.h"

/* Corange Functions */

void corange_init(const char* core_assets_path);
void corange_finish();

/* Audio and Graphics */

#include "graphics_manager.h"
#include "audio_manager.h"

/* Entities */

#include "entity_manager.h"

#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/physics_object.h"
#include "entities/instance_object.h"
#include "entities/camera.h"
#include "entities/light.h"
#include "entities/landscape.h"

/* Assets */

#include "asset_manager.h"

#include "assets/animation.h"
#include "assets/skeleton.h"
#include "assets/renderable.h"
#include "assets/collision_body.h"
#include "assets/font.h"
#include "assets/material.h"
#include "assets/shader.h"
#include "assets/texture.h"
#include "assets/image.h"
#include "assets/sound.h"
#include "assets/terrain.h"

/* UI */

#include "ui_manager.h"

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
