#include "window.hpp"

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
