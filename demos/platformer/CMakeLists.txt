cmake_minimum_required(VERSION 3.13)
project(platformer)

set(CMAKE_C_STANDARD 99)

add_executable(platformer src/platformer.c
                          src/character.c
                          src/coin.c
                          src/level.c)

include_directories (include)

target_link_libraries(platformer ${LIBCORANGE})
target_link_libraries(platformer GL)
target_link_libraries(platformer SDL2)
target_link_libraries(platformer SDL2_net)
target_link_libraries(platformer SDL2_mixer)
target_link_libraries(platformer m)
