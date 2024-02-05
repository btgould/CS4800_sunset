#include "input.hpp"
#include "application/application.hpp"
#include "imgui.h"
#include <GLFW/glfw3.h>

bool Input::isKeyPressed(int key) {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();
	bool imguiCapture = ImGui::GetIO().WantCaptureKeyboard;

	return glfwGetKey(window, key) && !imguiCapture;
}

bool Input::isMouseButtonPressed(int button) {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();
	bool imguiCapture = ImGui::GetIO().WantCaptureMouse;

	return glfwGetMouseButton(window, button) && !imguiCapture;
}

glm::vec2 Input::getMousePos() {
	GLFWwindow* window = Application::get().getWindow().getNativeWindow();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return {(float) xpos, (float) ypos};
}
