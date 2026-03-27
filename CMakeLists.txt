cmake_minimum_required(VERSION 3.14)

project(nbodysim)
# This is to download our dependencies
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES_FILES src/glad.c src/main.cpp src/sim.cpp src/render.cpp src/input.cpp src/container/octree.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TARGET nbody)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3)

FetchContent_MakeAvailable(glfw glm)

include_directories(include)
add_executable("${TARGET}" "${SOURCES_FILES}")

target_link_libraries("${TARGET}" glfw glm)
