include(CMakeParseArguments)

macro(demo)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCES ASSETS LIBS)

  set(SOURCES)
  set(PLATFORM_LIBS)

  find_package(SDL REQUIRED)
  find_package(OpenGL REQUIRED)

  if(WIN32)
    # TODO: add platform code
  elseif(APPLE)
    # TODO: add platform code
  elseif(UNIX)
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
  endif()

  cmake_parse_arguments(DEMO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(SOURCE ${DEMO_SOURCES})
    set(SOURCES ${SOURCES} "src/${SOURCE}")
  endforeach(SOURCE)

  project(${DEMO_NAME}})
  include_directories(include ../../engine/include)
  include_directories(${SDL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
  add_executable(${DEMO_NAME} ${SOURCES})
  target_link_libraries(${DEMO_NAME} ${SDL_LIBRARY} ${OPENGL_LIBRARIES} ${PLATFORM_LIBS} engine m ${DEMO_LIBS})

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
