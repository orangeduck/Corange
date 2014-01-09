include(CMakeParseArguments)

macro(demo)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCES ASSETS LIBS)
  set(SOURCES)

  cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(SOURCE ${DEMO_SOURCES})
    set(SOURCES ${SOURCES} "src/${SOURCE}")
  endforeach(SOURCE)

  project(${DEMO_NAME}})
  include_directories(include)
  include_directories(../../engine/include)
  add_executable(${DEMO_NAME} ${SOURCES})
  target_link_libraries(${DEMO_NAME} SDL engine m GL GLU X11 Xxf86vm pthread Xrandr Xi ${DEMO_LIBS})

  foreach(ASSET ${DEMO_ASSETS})
    file(COPY ${ASSET} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  endforeach(ASSET)
endmacro(demo)

add_subdirectory(renderers)
add_subdirectory(platformer)
add_subdirectory(scotland)
add_subdirectory(sea)
add_subdirectory(tessellation)
add_subdirectory(lut_gen)
add_subdirectory(metaballs)
add_subdirectory(noise)
