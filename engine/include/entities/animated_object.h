/**
*** :: Animated Object ::
***
***   A skeletally animated object
***   holds information about current animation time
***   Builds a new pose skeleton object on update
***
**/

#ifndef animated_object_h
#define animated_object_h

#include "cengine.h"
#include "casset.h"

#include "assets/skeleton.h"

typedef struct {
  
  vec3 position;
  vec3 scale;
  vec4 rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  float animation_time;

  asset_hndl renderable;
  asset_hndl animation;
  asset_hndl skeleton;
    
  skeleton* pose;
  
} animated_object;

animated_object* animated_object_new();
void animated_object_delete(animated_object* ao);

void animated_object_load_skeleton(animated_object* ao, asset_hndl ah);

void animated_object_update(animated_object* ao, float timestep);

#endif
