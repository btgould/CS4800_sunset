#include "input.hpp"
#include "application/application.hpp"
#include <GLFW/glfw3.h>

bool Input::isKeyPressed(int key) {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();

	return glfwGetKey(window, key);
}

bool Input::isMouseButtonPressed(int button) {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();

	return glfwGetMouseButton(window, button);
}

glm::vec2 Input::getMousePos() {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return {(float) xpos, (float) ypos};
}
