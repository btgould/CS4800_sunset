#include "device.hpp"
#include "pipeline.hpp"
#include "renderer/IndexBuffer.hpp"
#include "renderer/renderer.hpp"
#include "spdlog/common.h"

#include <stdexcept>

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "instance.hpp"
#include "window.hpp"

class HelloTriangleApplication {
  public:
	HelloTriangleApplication()
		: m_window(GLFWWindow("Vulkan")), m_instance(m_window), m_device(m_instance),
		  m_renderer(m_instance, m_device, m_window),
		  m_vertexBuffer(m_device, {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	                                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	                                {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	                                {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}}),
		  m_indexBuffer(m_device, {0, 1, 2, 2, 3, 0}) {}

	~HelloTriangleApplication() = default;

	void run() {
		while (!m_window.shouldClose()) {
			m_window.pollEvents();

			m_renderer.beginScene();
			m_renderer.draw(m_vertexBuffer, m_indexBuffer);
			m_renderer.endScene();
			/* m_pipeline.drawFrame(); */
		}

		m_device.flush();
	}

  private:
	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanDevice m_device;
	VulkanRenderer m_renderer;
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
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
