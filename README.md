# CS 4500 Project

## Requirements

Vulkan dev tools:

-   Ubuntu / Debian: `sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools`
-   Fedora: `sudo dnf install vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel`
-   Arch: `sudo pacman -S vulkan-devel`

GLFW: (windowing library)

-   Ubuntu / Debian: `sudo apt install libglfw3-dev`
-   Fedora: `sudo dnf install glfw-devel`
-   Arch: `sudo pacman -S glfw-wayland # glfw-x11 for X11 users`

GLM: (linear algebra library)

-   Ubuntu / Debian: `sudo apt install libglm-dev`
-   Fedora: `sudo dnf install glm-devel`
-   Arch: `sudo pacman -S glm`

GLSLC: (shader compiler)

-   Ubuntu / Debian: [Install from source](https://github.com/google/shaderc/blob/main/downloads.md)
-   Fedora: `sudo dnf install glslc`
-   Arch: `sudo pacman -S shaderc`

## Building

This project builds using CMake.
Only linux is supported, and there are no plans to support any other environments.
A `.toggletasks` file is included for the nvim extension, but it should be human readable enough to find the exact build commands needed for other IDEs.
