cmake_minimum_required(VERSION 3.8)

project(CS4800_Sunset)

# Global CMake config
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Config for dependencies that need to be build
add_subdirectory(vendor/spdlog)

# Config for main executable
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS src/*.cpp) 
add_executable(sunset ${SOURCE_FILES})

target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/spdlog")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/stb")

target_link_libraries(sunset glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
target_link_libraries(sunset spdlog)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
	target_compile_definitions(sunset PRIVATE SUNSET_DIST)
elseif(${CMAKE_BUILD_TYPE} STREQUAL RelWithDebugInfo)
	target_compile_definitions(sunset PRIVATE SUNSET_RELEASE)
else()
	target_compile_definitions(sunset PRIVATE SUNSET_DEBUG)
endif()

