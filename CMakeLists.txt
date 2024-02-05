cmake_minimum_required(VERSION 3.8)

project(CS4800_Sunset)

# Useful vars 
set(IMGUI_DIR  "${PROJECT_SOURCE_DIR}/vendor/imgui")

# Global CMake config
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Config for dependencies that need to be build
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/tinyobjloader)

# Config for main executable
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS src/*.cpp) 
add_executable(sunset ${SOURCE_FILES})

target_sources(sunset PRIVATE ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)

target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/spdlog")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/stb")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/tinyobjloader")
target_include_directories(sunset PRIVATE "${PROJECT_SOURCE_DIR}/vendor/imgui")

target_link_libraries(sunset glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
target_link_libraries(sunset spdlog)
target_link_libraries(sunset tinyobjloader)

target_compile_definitions(sunset PRIVATE GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
	target_compile_definitions(sunset PRIVATE SUNSET_DIST)
elseif(${CMAKE_BUILD_TYPE} STREQUAL RelWithDebugInfo)
	target_compile_definitions(sunset PRIVATE SUNSET_RELEASE)
else()
	target_compile_definitions(sunset PRIVATE SUNSET_DEBUG)
endif()

