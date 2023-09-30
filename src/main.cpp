#include "pipeline.hpp"
#include "spdlog/common.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "instance.hpp"
#include "window.hpp"

class HelloTriangleApplication {
  public:
	HelloTriangleApplication()
		: m_window(GLFWWindow("Vulkan")), m_instance(m_window), m_pipeline(m_instance) {}

	~HelloTriangleApplication() = default;

	void run() {
		while (!m_window.shouldClose()) {
			glfwPollEvents();
			m_pipeline.drawFrame();
		}

		m_pipeline.flush();
	}

  private:
	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanPipeline m_pipeline;
};

int main() {
	Log::Init(spdlog::level::info);
	HelloTriangleApplication app;

	try {
		PROFILE_BEGIN_SESSION("Runtime", "Runtime_Profile.json");
		app.run();
		PROFILE_END_SESSION();
	} catch (const std::exception& e) {
		LOG_ERROR("{0}", e.what());
		PROFILE_END_SESSION();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
