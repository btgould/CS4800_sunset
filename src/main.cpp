#include "spdlog/common.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdlib>

#include "util/log.hpp"
#include "instance.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
  public:
	void run() {

		initWindow();
		m_instance = VulkanInstance();
		mainLoop();
		cleanup();
	}

  private:
	GLFWwindow* m_window;
	VulkanInstance m_instance;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		m_instance.cleanup();

		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
};

int main() {
	Log::Init(spdlog::level::info);
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception& e) {
		LOG_ERROR("{0}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
