cmake_minimum_required(VERSION 3.13)
project(corange)

set(CMAKE_C_STANDARD 99)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library (corange STATIC src/ui/ui_style.c
                              src/ui/ui_toast.c
                              src/entities/physics_object.c
                              src/data/vertex_list.c
                              src/ui/ui_text.c
                              src/ui/ui_button.c
                              src/assets/sound.c
                              src/assets/terrain.c
                              src/data/vertex_hashtable.c
                              src/assets/renderable.c
                              src/assets/cmesh.c
                              src/entities/landscape.c
                              src/casset.c
                              src/assets/texture.c
                              src/data/int_list.c
                              src/data/list.c
                              src/assets/material.c
                              src/entities/animated_object.c
                              src/ui/ui_option.c
                              src/data/spline.c
                              src/caudio.c
                              src/entities/instance_object.c
                              src/cphysics.c
                              src/data/dict.c
                              src/entities/particles.c
                              src/ui/ui_slider.c
                              src/ui/ui_listbox.c
                              src/entities/static_object.c
                              src/ui/ui_browser.c
                              src/centity.c
                              src/ui/ui_rectangle.c
                              src/ui/ui_dialog.c
                              src/data/randf.c
                              src/assets/lang.c
                              src/ui/ui_spinner.c
                              src/assets/animation.c
                              src/assets/image.c
                              src/assets/effect.c
                              src/assets/skeleton.c
                              src/assets/music.c
                              src/assets/config.c
                              src/SDL2/SDL_local.c
                              src/cui.c
                              src/cgraphics.c
                              src/assets/font.c
                              src/corange.c
                              src/cjoystick.c
                              src/rendering/sky.c
                              src/entities/camera.c
                              src/cengine.c
                              src/entities/light.c
                              src/cnet.c
                              src/ui/ui_textbox.c
                              src/assets/shader.c
                              src/rendering/renderer.c)

include_directories (include include/assets include/data include/entities include/rendering include/SDL2 include/ui)

find_library(LIBCORANGE corange ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_subdirectory(demos/platformer)