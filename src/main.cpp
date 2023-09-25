#include "pipeline.hpp"
#include "spdlog/common.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "util/log.hpp"
#include "instance.hpp"
#include "window.hpp"

class HelloTriangleApplication {
  public:
	HelloTriangleApplication() : m_window(GLFWWindow("Vulkan")), m_instance(m_window), m_pipeline(m_instance) {}

	void run() {
		while (!m_window.shouldClose()) {
			glfwPollEvents();
		}
	}

  private:
	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanPipeline m_pipeline;
};

int main() {
	Log::Init(spdlog::level::trace);
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception& e) {
		LOG_ERROR("{0}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
