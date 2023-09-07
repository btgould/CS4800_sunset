cmake_minimum_required(VERSION 3.0)

project(CS4800_Sunset)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}/src")

link_libraries(glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
add_executable(sunset ${PROJECT_SOURCE_DIR}/src/main.cpp)
