#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWWindow {
  public:
	GLFWWindow(std::string name, uint32_t width = 800, uint32_t height = 600);
	~GLFWWindow();

	GLFWWindow(const GLFWWindow&) = delete;
	GLFWWindow& operator=(const GLFWWindow&) = delete;

	inline bool shouldClose() { return glfwWindowShouldClose(m_window); }

	void createSurface(VkInstance instance, VkSurfaceKHR* surface);
	/**
	 * @brief Gets the size of the framebuffer in pixels
	 *
	 * @return Extent describing window size in pixels
	 */
	const VkExtent2D getFramebufferSize() const;

  private:
	GLFWwindow* m_window;

	std::string m_name;
	uint32_t m_width;
	uint32_t m_height;
};