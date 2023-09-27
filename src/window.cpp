#include "window.hpp"
#include <stdexcept>

GLFWWindow::GLFWWindow(std::string name, uint32_t width, uint32_t height)
	: m_name(name), m_width(width), m_height(height) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
}

GLFWWindow::~GLFWWindow() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void GLFWWindow::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
	if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

const VkExtent2D GLFWWindow::getFramebufferSize() const {
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
