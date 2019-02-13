cmake_minimum_required(VERSION 3.10)
project(corange LANGUAGES C)

set(CMAKE_SKIP_BUILD_RPATH TRUE)

add_definitions(-DHOST -Wall -Wextra -Wshadow -Wformat=2 -Wfloat-equal -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -D_FORTIFY_SOURCE=2 -Wno-unused)

#set (SANITIZE_FLAGS "-fsanitize=address  -fsanitize=leak -g -fno-omit-frame-pointer -fsanitize=undefined -fstack-protector")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZE_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SANITIZE_FLAGS}")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

file(GLOB SRC src/*.c)
file(GLOB ASSETS src/assets/*.c)
file(GLOB DATA src/data/*.c)
file(GLOB ENTITIES src/entities/*.c)
file(GLOB RENDERING src/rendering/*.c)
file(GLOB SDL2 src/SDL2/*.c)
file(GLOB UI src/ui/*.c)

source_group("" FILES ${SRC})
source_group("assets" FILES ${ASSETS})
source_group("data" FILES ${DATA})
source_group("entities" FILES ${ENTITIES})
source_group("rendering" FILES ${RENDERING})
source_group("SDL2" FILES ${SDL2})
source_group("SDL2" FILES ${UI})

set( CORANGE_SOURCES ${SRC} ${ASSETS} ${DATA} ${ENTITIES} ${RENDERING} ${SDL2} ${UI})

add_library(${PROJECT_NAME} SHARED ${CORANGE_SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} GL SDL2 SDL2_mixer SDL2_net)

add_subdirectory(demos)


