cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-std=c99 -Wall")

add_subdirectory(engine)
add_subdirectory(demos)

file(COPY core_assets DESTINATION ${CMAKE_BINARY_DIR})
