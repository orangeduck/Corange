project(engine)

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(${SDL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

set(SOURCES
  src/data/int_list.c
  src/data/list.c
  src/data/vertex_list.c
  src/data/dict.c
  src/data/vertex_hashtable.c
  src/assets/image.c
  src/assets/texture.c
  src/assets/sound.c
  src/assets/font.c
  src/assets/skeleton.c
  src/assets/terrain.c
  src/assets/renderable.c
  src/assets/animation.c
  src/assets/collision_body.c
  src/assets/shader.c
  src/assets/material.c
  src/casset.c
  src/cgraphics.c
  src/centity.c
  src/ui/ui_text.c
  src/ui/ui_button.c
  src/ui/ui_spinner.c
  src/ui/ui_rectangle.c
  src/enitites/landscape.c
  src/enitites/camera.c
  src/enitites/static_object.c
  src/enitites/animated_object.c
  src/enitites/light.c
  src/enitites/physics_object.c
  src/enitites/instance_object.c
  src/cjoystick.c
  src/SDL/SDL_local.c
  src/cengine.c
  src/rendering/deferred_renderer.c
  src/rendering/shadow_mapper.c
  src/rendering/depth_mapper.c
  src/rendering/forward_renderer.c
  src/cui.c
  src/caudio.c
  src/corange.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${SDL_LIBRARY} ${OPENGL_LIBRARIES})
